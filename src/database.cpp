#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "database.h"
#include "utils.h"

#define LINE_MAX_SIZE 10000

namespace fs = filesystem;
using namespace std;

Database::Database()
{

}

size_t Database::get_useflag_id(const string &useflag_str, bool create_ids)
{
    /* Returns the id of the useflag given by useflag_str
     * if create_ids = true, it creates an id if it doesn't exist already
     * if create_ids = false and the string hasn't been encountered before, returns npos
     */

    // TODO: figure out how to use string_view for performance
    //       c.f. https://en.cppreference.com/w/cpp/container/unordered_map/find

    size_t useflag_id = npos;
    auto use_flag_it = useflag_str_to_id.find(useflag_str);
    if(use_flag_it == useflag_str_to_id.end())
    {
        if(create_ids)
        {
            useflag_id = useflag_str_to_id.size();
            useflag_str_to_id.insert(make_pair(useflag_str, useflag_id));
        }
    }
    else useflag_id = use_flag_it->second;

    return useflag_id;
}

std::unordered_map<std::size_t, bool> Database::parse_useflags(const string_view &useflags_str, bool default_state, bool create_ids)
{
    auto start_it = useflags_str.begin(), end_it = useflags_str.begin();

    std::unordered_map<std::size_t, bool> parsed_useflags;

    bool found_useflag = false;
    bool state = default_state;
    while (start_it != useflags_str.end())
    {
        if(not found_useflag)
        {
            if(*start_it == ' ')
            {
                start_it++;
                continue;
            }

            found_useflag = true;
            if(*start_it == '+')
            {
                state = true;
                start_it++;
            }
            else if(*start_it == '-')
            {
                state = false;
                start_it++;
            }

            end_it = start_it;
        }
        else
        {
            if(*end_it == ' ' or end_it == useflags_str.end())
            {
                // We isolated a useflag between start_it and end_it
                // TODO: string(string_view()) is counter-productive for performance
                //       figure out how to do it with Hash::is_transparent and KeyEqual::is_transparent
                //       c.f. https://en.cppreference.com/w/cpp/container/unordered_map/find
                size_t useflag_id = get_useflag_id(string(string_view(start_it, end_it)), create_ids);
                if(useflag_id == npos)
                    cout << "This useflag doesn't exist: " + string(string_view(start_it, end_it)) << endl;

                // add flag id and its state to the list
                parsed_useflags.insert(make_pair(useflag_id, state));

                // reset the iterators and the state boolean
                start_it = end_it == useflags_str.end() ?  end_it : end_it + 1;
                state = default_state;
                found_useflag = false;
            }
            else end_it++;
        }
    }

    return parsed_useflags;
}

void Database::account_for_global_useflags()
{
    const string &global_useflags_str = exec("portageq envvar USE") + " " + exec("portageq envvar IUSE_IMPLICIT");

    // All use flags are contained in ebuilds in md5-cache, do not create new ones from profile
    auto global_useflags = parse_useflags(global_useflags_str, true, false);
    VersionConstraint constraint;
    constraint.type = VersionConstraint::Type::NONE;

    for(size_t i = 0 ; i < pkgs.size(); i++)
        pkgs[i].update_useflags_with_constraints(constraint, global_useflags);
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

    char line[LINE_MAX_SIZE];
    size_t first_space_char;

    for(const auto &path: userflag_file_paths)
    {
        fstream file(path, ios::in);
        if(not file.is_open())
            throw runtime_error("Couldn't open file" + path.string());

        while(file.getline(line, LINE_MAX_SIZE, '\n'))
        {
            string_view line_str_view(line);
            // remove spaces at the beginning
            line_str_view.remove_prefix(min(line_str_view.find_first_not_of(' '), line_str_view.size()));

            if(line_str_view.empty() or line_str_view.starts_with('#'))
                continue;

            // find the first space that separates the package specification from the use flags
            // e.g.      >=app-misc/foo-1.2.3 +foo -bar
            //           first space here:   ^

            first_space_char = line_str_view.find(' ');
            if(first_space_char == string_view::npos)
                throw runtime_error("In file: " + path.string() + "\n Cannot parse line: " + string(line));

            // create a view on the package constraint str ">=app-misc/foo-1.2.3" then parse it
            string_view pkg_constraint_str_view(line_str_view);
            pkg_constraint_str_view.remove_suffix(line_str_view.size() - first_space_char);
            auto pkg_constraint = parse_pkg_constraint(pkg_constraint_str_view, pkgs);

            if(pkg_constraint.pkg_id == pkgs.npos)
                continue;

            // create a view on the useflags "+foo -bar" and parse it
            string_view useflags_str_view(line_str_view);
            useflags_str_view.remove_prefix(pkg_constraint_str_view.size());
            auto useflags = parse_useflags(useflags_str_view, true);

            // give that to the relevant package so it updates its ebuilds
            pkgs[pkg_constraint.pkg_id].update_useflags_with_constraints(pkg_constraint.ver, useflags);
        }
    }

}

void Database::add_dependencies_to_ebuild(const string &dep_string, Ebuild &ebuild)
{

}

void Database::populate_from_cache_dir(string path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))    
        throw runtime_error("Path is not a directory");

    string pkg_group, pkg_namever, pkg_name, pkg_ver;
    size_t split_pos;

    char ebuild_line[LINE_MAX_SIZE];

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_path))
    {
        if(not (entry.is_regular_file() and entry.path().parent_path().parent_path() == cache_path))
            continue;

        pkg_namever = entry.path().filename(); // we use filename here because the cache files do not contain any extension
        pkg_group = entry.path().parent_path().stem();

        split_pos = pkg_namever_split_pos(pkg_namever);

        pkg_name = pkg_namever.substr(0, split_pos);
        pkg_ver = pkg_namever.substr(split_pos+1);

        Package &pkg = pkgs[pkg_group + "/" + pkg_name]; // created if doesn't exist
        auto &ebuild = pkg.add_version(pkg_ver);

        fstream file(entry.path(), ios::in);
        if(not file.is_open())
            throw runtime_error("Couldn't open file" + entry.path().string());

        while(file.getline(ebuild_line, LINE_MAX_SIZE, '\n'))
        {
            string_view ebuild_line_view(ebuild_line);
            if(ebuild_line_view.starts_with("IUSE"))
            {
                // +5 to remove the "IUSE=" in the beginning
                // IUSE useflags are by default disabled afaik
                auto flag_states = parse_useflags(string_view(ebuild_line+5), false, true);
                ebuild.add_useflags(flag_states);
            }
        }
    }

    account_for_global_useflags();
    account_for_user_useflags();
}
