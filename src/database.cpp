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

Database::Database(): pkgs(make_shared<NamedVector<Package>>()), useflags(make_shared<NamedVector<string>>()), parser(make_shared<Parser>(pkgs, useflags))
{
}

void Database::load_ebuilds(const std::string &path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))
        throw runtime_error("Path is not a directory");

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
        pkg.add_version(pkg_ver, read_file_lines(entry.path()));
    }
}

void Database::load_profile_settings()
{
    fs::path profile_symlink("/etc/portage/make.profile");
    if(not fs::is_symlink(profile_symlink))
        throw runtime_error("/etc/portage/make.profile doesn't exist or isn't a symlink");

    fs::path profile = fs::canonical(profile_symlink);
    cout << "Absolute path of the profile " << profile.string() << endl;

    vector<fs::path> profile_tree = {profile};
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

    cout << "package masks:" << endl;
//    vector<fs::path>              use_force,     use_mask,     use_stable_force,     use_stable_mask;
//    vector<fs::path> pkg_use, pkg_use_force, pkg_use_mask, pkg_use_stable_force, pkg_use_stable_mask;
//    vector<fs::path> pkg_mask;

//    unordered_map<string, UseflagStates> global_use_toggles =
//    { {"use.force", UseflagStates()}, {"use.mask", UseflagStates()}, {"use.stable.force", UseflagStates()}, {"use.stable.mask", UseflagStates()} };

//    unordered_map<string, vector<PkgUseToggles>> pkg_use_toggles =
//     { {"package.use.force", vector<PkgUseToggles>()},        {"package.use.mask", vector<PkgUseToggles>()},
//       {"package.use.stable.force", vector<PkgUseToggles>()}, {"package.use.stable.mask", vector<PkgUseToggles>()} };

    vector<PackageConstraint> pkg_masks;

    // TODO : continue here

    for(auto it = profile_tree.rbegin() ; it != profile_tree.rend() ; it++)
    {
        cout << "##################################################" << endl;
        cout << "Profile directory: " << it->string() << endl;

        for(const auto &path: get_regular_files(it->string() + "/use.force"))
        {
            for(string_view line: read_file_lines(path))
            {
                cout << "use.force line: " << string(line) << endl;
                bool unforce = false;
                if(line.starts_with('-'))
                {
                    cout << "                 Unforced" << endl;
                    line.remove_prefix(1);
                    unforce = true;
                }
                size_t flag_id = useflags->id_of(line);
                if(flag_id != npos)
                {
                    cout << "                 Flag id: " << flag_id << endl;
                    if(unforce)
                        forced_flags.erase(flag_id);
                    else forced_flags.insert(flag_id);
                }
            }
        }
    }
}

void Database::account_for_global_useflags()
{
    string global_useflags_str = exec("portageq envvar USE");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    global_useflags_str += " " + exec("portageq envvar IUSE_IMPLICIT");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    // All use flags are contained in ebuilds in md5-cache, do not create new ones from profile
    auto global_useflags = parser->parse_useflags(global_useflags_str, true, false);
    VersionConstraint constraint;
    constraint.type = VersionConstraint::Type::NONE;

    for(size_t i = 0 ; i < pkgs->size(); i++)
        (*pkgs)[i].update_useflags_with_constraints(constraint, global_useflags);
}

void Database::account_for_user_useflags()
{
    vector<fs::path> userflag_file_paths;

    fs::path user_useflags_path("/etc/portage/package.use");
    if(fs::is_directory(user_useflags_path))
    {
        for(fs::directory_entry const& entry: fs::recursive_directory_iterator(user_useflags_path))
        {
            if(entry.is_regular_file())
                userflag_file_paths.emplace_back(entry.path());
        }
    }
    else if(fs::is_regular_file(user_useflags_path))
    {
        userflag_file_paths.emplace_back(user_useflags_path);
    }
    else throw runtime_error("/etc/portage/package.use is weird");

    for(const auto &path: userflag_file_paths)
    {
        for(string_view line_str_view: read_file_lines(path))
        {
            const auto &pkg_useflag_toggles = parser->parse_pkguse_line(line_str_view);

            if(not pkg_useflag_toggles.first.is_valid)
                continue;

            // give that to the relevant package so it updates its ebuilds
            (*pkgs)[pkg_useflag_toggles.first.pkg_id].update_useflags_with_constraints(pkg_useflag_toggles.first.ver, pkg_useflag_toggles.second);
        }
    }
}

void Database::parse_iuse()
{
    for(auto &pkg: *pkgs)
        pkg.parse_iuse();
}

void Database::parse_deps()
{
    for(auto &pkg: *pkgs)
        pkg.parse_deps();
}

void Database::populate(const std::string &overlay_cache_path)
{
    auto start = high_resolution_clock::now();

    load_ebuilds(overlay_cache_path);
    parse_iuse();
    parse_deps();

    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "It took " << duration.count() << "ms to read ::gentoo cache" << endl;

    load_profile_settings();
}
