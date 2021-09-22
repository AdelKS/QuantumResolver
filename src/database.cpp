#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "database.h"
#include "utils.h"

#define EBUILD_LINE_MAX_SIZE 10000

namespace fs = filesystem;
using namespace std;

pair<string,string> get_name_and_ver(const string &name_ver)
{
    bool found = false, found_second = false;
    size_t last_dash = 0, before_last_dash = 0;
    for(size_t i = 0 ; i < name_ver.size() ; i++)
        if(name_ver[i] == '-')
        {
            if(found_second)
                cout << "many dashes!" << endl;
            if(found)
                found_second = true;
            found = true;
            before_last_dash = last_dash;
            last_dash = i;
        }

    if(not found)
        throw "Error splitting " + name_ver;
    else if(not found_second)
        return pair<string,string>(name_ver.substr(0, last_dash), name_ver.substr(last_dash+1));
    else
    {
        if(last_dash + 1 == name_ver.size())
            throw "Error splitting " + name_ver;
        else if(name_ver[last_dash+1] == 'r') // this a revision number
            return pair<string,string>(name_ver.substr(0, before_last_dash), name_ver.substr(before_last_dash+1));
        else return pair<string,string>(name_ver.substr(0, last_dash), name_ver.substr(last_dash+1));
    }

}



Database::Database()
{

}

void Database::add_use_flags_to_ebuild(const string &uses_string, Package &pkg, const string &pkg_group_namever)
{
    auto pkg_use_flags = split_string(uses_string, vector<string>({string(" ")})); // +5 to remove the "IUSE=" in the beginning
    Ebuild &ebuild = pkg.get_ebuild(pkg_group_namever);

    int use_flag_id;
    UseFlagDefaults flag_defaults;
    string use_str;

    for(const auto &use_split: pkg_use_flags)
    {
        // Handle defaults for the flag, if it has one. e.g. +espeak -something
        if(use_split.second[0] == '+' or use_split.second[0] == '-')
        {
            // the flag comes with a default
            use_str = use_split.second.substr(1);
            flag_defaults.has_default = true;
            flag_defaults.default_positive = use_str[0] == '+';
        }
        else
        {
            // the flag doesn't come with a default
            use_str = use_split.second;
            flag_defaults.has_default = false;
            flag_defaults.default_positive = true;
        }

        auto use_flag_it = iuse_to_id.find(use_str);
        if(use_flag_it == iuse_to_id.end())
        {
            use_flag_id = iuse_to_id.size();
            iuse_to_id.insert(use_flag_it, std::pair<string, int>(use_str, iuse_to_id.size()));
        }
        else use_flag_id = use_flag_it->second;

        ebuild.add_use_flag(use_flag_id, flag_defaults);
    }
}

void Database::populate_from_cache_dir(string path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))
    {
        throw "Path is not a directory";
    }

    string pkg_group, pkg_namever, pkg_name, pkg_ver;
    int pkg_index;

    char ebuild_line[EBUILD_LINE_MAX_SIZE];

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_path))
    {
        if(entry.is_regular_file()
                and entry.path().parent_path().parent_path() == cache_path)
        {
            pkg_namever = entry.path().filename(); // we use filename here because the cache files do not contain any extension
            pkg_group = entry.path().parent_path().stem();

            const auto &name_and_ver = get_name_and_ver(pkg_namever);

            pkg_name = name_and_ver.first;
            pkg_ver = name_and_ver.second;

            pkg_index = pkgs.index_of(pkg_group + "/" + pkg_name);
            if(pkg_index == pkgs.npos)
                pkg_index = pkgs.new_object(pkg_group + "/" + pkg_name);

            pkgs[pkg_index].add_version(pkg_ver);
            pkg_group_namever_to_pkg_id[pkg_group + "/" + pkg_namever] = pkg_index;

            fstream file(entry.path(), ios::in);
            if(not file.is_open())
                throw "Couldn't open file" + entry.path().string();

            while(file.getline(ebuild_line, EBUILD_LINE_MAX_SIZE, '\n'))
            {
                if(starts_with(ebuild_line, "IUSE", 4))
                {
                    add_use_flags_to_ebuild(string(ebuild_line+5), pkgs[pkg_index], pkg_group + "/" + pkg_namever);
                    cout << "Done adding use flags" << endl;
                }

            }
        }
    }
}
