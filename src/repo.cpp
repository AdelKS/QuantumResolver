#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <stdexcept>
#include <chrono>
#include <map>
#include <array>
using namespace std::chrono;

#include "repo.h"
#include "database.h"
#include "misc_utils.h"

using namespace std;
namespace fs = filesystem;

Repo::Repo(Database *db) : db(db)
{
//    auto start = high_resolution_clock::now();

    load_ebuilds("/var/db/repos/gentoo/metadata/md5-cache");
    load_installed_pkgs();
    load_package_useflag_settings();
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

void Repo::load_package_useflag_settings()
{
    const vector<pair<string, FlagAssignType>> pkguse_profile_files =
    {
        {"package.use", FlagAssignType::DIRECT},
        {"package.stable.use", FlagAssignType::STABLE_DIRECT},

        {"package.use.force", FlagAssignType::FORCE},
        {"package.use.stable.force", FlagAssignType::STABLE_FORCE},

        {"package.use.mask", FlagAssignType::MASK},
        {"package.use.stable.mask", FlagAssignType::STABLE_MASK},
    };

    cout << "Reading profile tree and forwarding per package use, force and mask flags to ebuilds" << endl;
    auto start = high_resolution_clock::now();

    for(const auto& profile_path : flatenned_profiles_tree)
        for(const auto& [profile_use_file, use_type]: pkguse_profile_files)
            for(const auto &path: get_regular_files(profile_path.string() + "/" + profile_use_file))
                for(string_view line: read_file_lines(path))
                {
                    const auto &[pkg_constraint, use_toggles] = db->parser.parse_pkguse_line(line);
                    if(pkg_constraint.pkg_id != pkgs.npos) //TODO : deal with assigning unexisting useflags
                        pkgs[pkg_constraint.pkg_id].assign_useflag_states(pkg_constraint, use_toggles, use_type);
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
        size_t pkg_id = pkgs.index_of(pkg_category_name);
        if(pkg_id == pkgs.npos)
        {
            pkg_id = pkgs.push_back(Package(pkg_category_name, db), pkg_category_name);
            pkgs.back().set_id(pkg_id);
        }

        Package &pkg = pkgs[pkg_id];
        pkg.add_repo_version(pkg_ver, entry.path());
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

            const size_t &pkg_id = pkgs.index_of(pkg_category + "/" + pkg_name);
            if(pkg_id != pkgs.npos)
                pkgs[pkg_id].add_installed_version(pkg_ver, pkg_namever_entry);
            else cout << pkg_category + "/" + pkg_name + " not in the ::gentoo repository" << endl;

        }
    }

    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

size_t Repo::get_pkg_id(const string_view &pkg_str)
{
    return pkgs.index_of(pkg_str);
}

Package& Repo::operator [] (PackageID pkg_id)
{
    return pkgs[pkg_id];
}

const string& Repo::get_pkg_groupname(size_t pkg_id)
{
    return pkgs[pkg_id].get_pkg_groupname();
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
