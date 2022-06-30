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
#include "string_utils.h"
#include "format_utils.h"
#include "file_utils.h"

using namespace std;
namespace fs = filesystem;

Repo::Repo(Database *db) : db(db)
{
    load_ebuilds("/var/db/repos/gentoo/metadata/md5-cache");
    load_installed_pkgs();
    load_system_packages();
    load_selected_packages();
    load_package_accept_keywords();
    load_package_useflag_settings();
}

void Repo::load_system_packages()
{
    for(const auto& profile_path : flatenned_profiles_tree)
    {
        fs::path packages(profile_path.string() + "/packages");
        if(fs::is_regular_file(packages))
        {
            for(string_view pkg_atom: read_file_lines(packages))
            {
                // if it doesn't start with '*' skip it
                // see 5.2.6 packages in the PMS
                if(not pkg_atom.starts_with('*'))
                    continue;
                pkg_atom.remove_prefix(1);

                auto constraint = db->parser.parse_pkg_constraint(pkg_atom);
                if(constraint.pkg_id != pkgs.npos)
                    system_pkgs.insert(constraint.pkg_id);
                else throw runtime_error(fmt::format("Could not recognise {} in {}", pkg_atom, profile_path.string()));
            }
        }
    }
}

void Repo::load_selected_packages()
{
    fs::path selected_packages("/var/lib/portage/world");
    if(fs::is_regular_file(selected_packages))
    {
        for(string_view pkg_atom: read_file_lines(selected_packages))
        {
            auto constraint = db->parser.parse_pkg_constraint(pkg_atom);
            if(constraint.pkg_id != pkgs.npos)
                selected_pkgs.insert(constraint.pkg_id);
            else fmt::print("There are no ebuilds for {} in ::gentoo\n", pkg_atom);
        }
    }
}

void Repo::load_package_accept_keywords()
{
    for(const auto &path: get_regular_files("/etc/portage/package.accept_keywords"))
        for(string_view line: read_file_lines(path))
        {
            const auto &[pkg_constraint, accept_keywords] = db->parser.parse_pkg_accept_keywords_line(line);
            if(pkg_constraint.pkg_id != pkgs.npos) //TODO : deal with assigning unexisting useflags
                pkgs[pkg_constraint.pkg_id].accept_keywords(pkg_constraint, accept_keywords);
        }
}

bool Repo::is_system_pkg(PackageID pkg_id) const { return system_pkgs.contains(pkg_id); };
bool Repo::is_selected_pkg(PackageID pkg_id) const { return selected_pkgs.contains(pkg_id); };

std::size_t Repo::get_pkg_id(const std::string_view &pkg_str) const { return pkgs.index_of(pkg_str);};
const std::string& Repo::get_pkg_groupname(std::size_t pkg_id) const { return pkgs[pkg_id].get_pkg_groupname(); };

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
