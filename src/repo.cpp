#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <stdexcept>

#include <chrono>
using namespace std::chrono;

#include "repo.h"
#include "database.h"

using namespace std;
namespace fs = filesystem;

Repo::Repo(Database *db) : db(db)
{
//    auto start = high_resolution_clock::now();

    load_ebuilds("/var/db/repos/gentoo/metadata/md5-cache");
//    load_installed_pkgs();
//    parse_deps();

//    auto end = high_resolution_clock::now();

//    cout << "#####################################################" << endl;
//    cout << "Total time : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}


template <class Map>
void update_map(Map &original, const Map &update)
{
    for(const auto &[key, val]: update)
    {
        original[key] = val;
    }
}

void Repo::load_masked_and_forced_useflags()
{

    vector<tuple<string, FlagAssignType, vector<PkgUseToggles>&>> pkguse_profile_files =
    {
        {"package.use", FlagAssignType::DIRECT, pkg_use},
        {"package.stable.use", FlagAssignType::STABLE_FORCE, pkg_use_stable},

        {"package.use.force", FlagAssignType::DIRECT, pkg_use_force},
        {"package.use.stable.force", FlagAssignType::STABLE_FORCE, pkg_use_stable},

        {"package.use.mask", FlagAssignType::MASK, pkg_use_mask},
        {"package.use.stable.mask", FlagAssignType::STABLE_MASK, pkg_use_stable_mask},
    };

    const auto &profile_tree = get_profiles_tree();

    cout << "Reading profile tree and forwarding per package use, force and mask flags to ebuilds" << endl;
    auto start = high_resolution_clock::now();

    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
    {
        for(auto &[profile_use_file, use_type, container]: pkguse_profile_files)
        {
            for(const auto &path: get_regular_files(it->string() + "/" + profile_use_file))
            {
                for(string_view line: read_file_lines(path))
                {
                    const auto &[pkg_constraint, use_toggles] = db->parser.parse_pkguse_line(line);
                    container.emplace_back(make_pair(pkg_constraint, use_toggles));
                    if(pkg_constraint.pkg_id != pkgs.npos) //TODO : deal with assigning unexisting useflags
                    {
                        pkgs[pkg_constraint.pkg_id].assign_useflag_states(pkg_constraint, use_toggles, use_type);
                    }
                }
            }

        }
    }
    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;

}


void Repo::load_ebuilds(const std::string &path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))
        throw runtime_error("Path is not a directory");

    cout << "Reading cached ebuilds from " + path << endl;
    auto start = high_resolution_clock::now();

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_path))
    {
        if(not (entry.is_regular_file() and entry.path().parent_path().parent_path() == cache_path))
            continue;

        const string &pkg_namever = entry.path().filename(); // we use filename here because the cache files do not contain any extension
        const string &pkg_category = entry.path().parent_path().stem();

        const size_t &split_pos = pkg_namever_split_pos(pkg_namever);

        const string &pkg_name = pkg_namever.substr(0, split_pos);
        const string &pkg_ver = pkg_namever.substr(split_pos+1);

        const string &pkg_category_name = pkg_category + "/" + pkg_name;
        size_t pkg_id = pkgs.id_of(pkg_category_name);
        if(pkg_id == pkgs.npos)
        {
            pkg_id = pkgs.push_back(Package(pkg_category_name, db), pkg_category_name);
            pkgs.back().set_id(pkg_id);
        }

        Package &pkg = pkgs[pkg_id];
        pkg.add_version(pkg_ver, entry.path());
    }

    auto end = high_resolution_clock::now();
    cout << "Loaded ebuilds in : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

void Repo::load_installed_pkgs()
{
    fs::path installed_pkgs_path("/var/db/pkg");
    if(not fs::is_directory(installed_pkgs_path))
        throw runtime_error("Path is not a directory");

    cout << "Reading installed ebuilds from " + installed_pkgs_path.string() << endl;
    auto start = high_resolution_clock::now();

    for(fs::directory_entry const& category_entry: fs::directory_iterator(installed_pkgs_path))
    {
        if(not category_entry.is_directory())
            continue;

        const string &pkg_category = category_entry.path().filename().string();

        for(fs::directory_entry const& pkg_namever_entry: fs::directory_iterator(category_entry.path()))
        {
            if(not pkg_namever_entry.is_directory())
                continue;

            const string &pkg_namever = pkg_namever_entry.path().filename().string();
            const size_t &split_pos = pkg_namever_split_pos(pkg_namever);

            const string &pkg_name = pkg_namever.substr(0, split_pos);
            const string &pkg_ver = pkg_namever.substr(split_pos+1);

            const size_t &pkg_id = pkgs.id_of(pkg_category + "/" + pkg_name);
            if(pkg_id != pkgs.npos)
            {
                const fs::path &useflags_path(pkg_namever_entry.path().string() + "/USE");
                string useflags;
                if(fs::is_regular_file(useflags_path))
                {
                    const auto &lines = read_file_lines(useflags_path);
                    if(lines.size() > 1)
                        cout << "Warning: " + useflags_path.string() + " contains more than one line" << endl;

                    pkgs[pkg_id].set_installed_version(pkg_ver, lines[0]);
                }
            }
            else
            {
                cout << pkg_category + "/" + pkg_name + " not in the ::gentoo repository" << endl;
            }
        }
    }

    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

size_t Repo::get_pkg_id(const string_view &pkg_str)
{
    return pkgs.id_of(pkg_str);
}

const string& Repo::get_pkg_groupname(size_t pkg_id)
{
    return pkgs[pkg_id].get_pkg_groupname();
}


void Repo::print_flag_states(const string &package_constraint_str)
{
    cout << "We got here!" << endl;
    PackageConstraint pkg_constraint = db->parser.parse_pkg_constraint(package_constraint_str);
    if(pkg_constraint.pkg_id == pkgs.npos)
    {
        cout << "Invalid atom: " << package_constraint_str << endl;
        return;
    }

    for(auto &ebuild_id: pkgs[pkg_constraint.pkg_id].get_matching_ebuilds(pkg_constraint))
        pkgs[pkg_constraint.pkg_id][ebuild_id].print_flag_states();
}

void Repo::parse_ebuild_metadata()
{
    for(auto &pkg: pkgs)
        pkg.parse_metadata();
}

void Repo::parse_deps()
{
    auto start = high_resolution_clock::now();

    for(auto &pkg: pkgs)
        pkg.parse_deps();

    auto end = high_resolution_clock::now();
    cout << "Parsing time : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}
