#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <stdexcept>

#include <chrono>
using namespace std::chrono;

#include "database.h"

using namespace std;
namespace fs = filesystem;

Database::Database():
    pkgs(make_shared<NamedVector<Package>>()),
    useflags(make_shared<NamedVector<string>>()),
    parser(make_shared<Parser>(pkgs, useflags))
{
}

void Database::load_ebuilds(const std::string &path)
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
        const string &pkg_group = entry.path().parent_path().stem();

        const size_t &split_pos = pkg_namever_split_pos(pkg_namever);

        const string &pkg_name = pkg_namever.substr(0, split_pos);
        const string &pkg_ver = pkg_namever.substr(split_pos+1);

        const string &pkg_group_name = pkg_group + "/" + pkg_name;
        size_t pkg_id = pkgs->id_of(pkg_group_name);
        if(pkg_id == pkgs->npos)
        {
            pkg_id = pkgs->emplace_back(Package(pkg_group_name, parser), pkg_group_name);
            pkgs->back().set_id(pkg_id);
        }

        Package &pkg = (*pkgs)[pkg_id];
        pkg.add_version(pkg_ver, entry.path());
    }

    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

template <class Map>
void update_map(Map &original, const Map &update)
{
    for(const auto &[key, val]: update)
    {
        original[key] = val;
    }
}

void Database::load_profile_settings()
{
    fs::path profile_symlink("/etc/portage/make.profile");
    if(not fs::is_symlink(profile_symlink))
        throw runtime_error("/etc/portage/make.profile doesn't exist or isn't a symlink");

    fs::path profile = fs::canonical(profile_symlink);
    cout << "Absolute path of the profile " << profile.string() << endl;

    cout << "Populating profile tree" << endl;
    auto start = high_resolution_clock::now();

    vector<fs::path> profile_tree = {"/etc/portage", profile};
    deque<fs::path> explore_queue = {profile};

    while(not explore_queue.empty())
    {
        const fs::path curr_profile = explore_queue.back(); explore_queue.pop_back();

        const fs::path &parent_file_path(curr_profile.string() + "/parent");
        if(fs::is_regular_file(parent_file_path))
        {
            deque<fs::path> new_profiles;
            // Invert the list of explored profiles in the parent file
            // so the first line of "parent" is explored first
            for(const string &line: read_file_lines(parent_file_path))
            {
                const auto &new_profile = fs::canonical(fs::path(curr_profile.string() + "/" + line + "/"));
                new_profiles.push_front(new_profile);
            }
            for(const auto &profile: new_profiles)
            {
                profile_tree.push_back(profile);
                explore_queue.push_back(profile);
            }
        }
    }

    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;


    cout << "Getting global use flags" << endl;
    start = high_resolution_clock::now();

    // we cheat here and read portageeq to get the final form of USE
    //  TODO: handle this ourselves...
    string global_useflags_str = exec("portageq envvar USE");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;

    cout << "Forwarding them to each ebuild" << endl;
    start = high_resolution_clock::now();

    // Parse that and forward it to the ebuilds
    global_useflags = parser->parse_useflags(global_useflags_str, true, true);
    for(Package &pkg: *pkgs)
        for(Ebuild &ebuild: pkg.get_ebuilds())
            ebuild.assign_useflag_states(global_useflags);

    end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;


    cout << "Reading global forced and masked flags from profile tree" << endl;
    start = high_resolution_clock::now();

    vector<tuple<string, FlagAssignType, UseflagStates&>> profile_use_files_and_type =
    {
        {"use.force", FlagAssignType::FORCE, use_force},
        {"use.stable.force", FlagAssignType::STABLE_FORCE, use_stable_force},
        {"use.mask", FlagAssignType::MASK, use_mask},
        {"use.stable.mask", FlagAssignType::STABLE_MASK, use_stable_mask},
    };

    // Do global useflag overrides first
    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
    {
        fs::path make_defaults_path(it->string() + "/make.defaults");
        if(fs::is_regular_file(make_defaults_path))
        {
            cout<< "#############################" << endl;
            cout << make_defaults_path.string() << endl;
            for(auto &line: read_file_lines(make_defaults_path))
                cout << line << endl;
        }
        for(auto &[profile_use_file, use_type, container]: profile_use_files_and_type)
            for(const auto &path: get_regular_files(it->string() + "/" + profile_use_file))
                update_map(container, parser->parse_useflags(read_file_lines(path), true, true));
    }

    end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;


    cout << "Forwarding them to ebuilds" << endl;
    start = high_resolution_clock::now();

    // Forward these global flags to the ebuilds
    for(Package &pkg: *pkgs)
        for(Ebuild &ebuild: pkg.get_ebuilds())
            for(auto &[profile_use_file, use_type, container]: profile_use_files_and_type)
                ebuild.assign_useflag_states(container, use_type);

    end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;

    vector<pair<string, FlagAssignType>> pkguse_profile_files =
    {
        {"package.use", FlagAssignType::DIRECT},
        {"package.stable.use", FlagAssignType::STABLE_FORCE},
        {"package.use.mask", FlagAssignType::MASK},
        {"package.stable.use.mask", FlagAssignType::STABLE_MASK},
    };

    cout << "Reading profile tree and forwarding per package use, force and mask flags to ebuilds" << endl;
    start = high_resolution_clock::now();

    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
    {
        for(auto &[profile_use_file, use_type]: pkguse_profile_files)
            for(const auto &path: get_regular_files(it->string() + "/" + profile_use_file))
                for(string_view line: read_file_lines(path))
                {
                    const auto &[pkg_constraint, use_toggles] = parser->parse_pkguse_line(line);
                    if(pkg_constraint.is_valid)
                    {
                        (*pkgs)[pkg_constraint.pkg_id].assign_useflag_states(pkg_constraint, use_toggles, use_type);
                    }
                }
    }
    end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;
}

void Database::print_flag_states(const string &package_constraint_str)
{
    cout << "We got here!" << endl;
    PackageConstraint pkg_constraint = parser->parse_pkg_constraint(package_constraint_str);
    if(not pkg_constraint.is_valid)
    {
        cout << "Invalid atom: " << package_constraint_str << endl;
        return;
    }

    for(auto &ebuild_id: (*pkgs)[pkg_constraint.pkg_id].get_matching_ebuilds(pkg_constraint))
    {
        (*pkgs)[pkg_constraint.pkg_id][ebuild_id].print_flag_states();
    }
}

void Database::parse_ebuild_metadata()
{
    for(auto &pkg: *pkgs)
        pkg.parse_metadata();
}

void Database::parse_deps()
{
    for(auto &pkg: *pkgs)
        pkg.parse_deps();
}

void Database::populate(const std::string &overlay_cache_path)
{
    load_ebuilds(overlay_cache_path);
    load_profile_settings();
}
