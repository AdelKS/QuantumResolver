#include <filesystem>
#include <iostream>

#include "database.h"

namespace fs = filesystem;
using namespace std;

inline string get_version(const string &name_ver, const string &name)
{
    return name_ver.substr(name.size()+1, name_ver.size() - name.size()-1);
}


Database::Database()
{

}

void Database::populate_from_cache_dir(string path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))
    {
        throw "Path is not a directory";
    }

    string pkg_group, pkg_namever;

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_path))
    {
        if(entry.is_regular_file()
                and entry.path().parent_path().parent_path() == cache_path)
        {
            pkg_namever = entry.path().filename(); // we use filename here because the cache files do not contain any extension
            pkg_group = entry.path().parent_path().stem();

            const auto it = pkg_namever_to_id.find(pkg_group + "/" + pkg_namever);
            if(it != pkg_namever_to_id.end())
            {
                cout << "package found" << endl;
            }
            else throw "there's no ebuild that stisfies this package version: " + pkg_group + "/" + pkg_namever;
        }
    }
}

void Database::populate_from_overlay(string path)
{
    fs::path overlay_path(path);
    if(not fs::is_directory(overlay_path))
    {
        throw "Path is not a directory";
    }

    string pkg_group, pkg_name, pkg_ver, pkg_namever;
    int pkg_index;

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(overlay_path))
    {
        if(entry.is_regular_file()
                and entry.path().extension() == ".ebuild"
                and entry.path().parent_path().parent_path().parent_path() == overlay_path)
        {
            pkg_namever = entry.path().stem();
            pkg_name = entry.path().parent_path().stem();
            pkg_group = entry.path().parent_path().parent_path().stem();
            pkg_ver = get_version(pkg_namever, pkg_name);

            pkg_index = pkgs.index_of(pkg_group + "/" + pkg_name);
            if(pkg_index == pkgs.npos)
                pkg_index = pkgs.new_object(pkg_group + "/" + pkg_name);

            pkgs[pkg_index].add_version(pkg_ver);
            pkg_namever_to_id[pkg_group + "/" + pkg_namever] = pkg_index;
        }
    }
}
