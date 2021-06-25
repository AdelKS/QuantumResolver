#include "database.h"

Database::Database()
{

}

void Database::populate_from_cache_dir(string path)
{
    fs::path cache_dir(path);
    if(not fs::is_directory(cache_dir))
    {
        cout << "Path is not a directory" << endl;
    }

//    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_dir))
//    {
//    }
}

void Database::populate_from_overlay(string path)
{
    fs::path overlay_path(path);
    if(not fs::is_directory(overlay_path))
    {
        cout << "Path is not a directory" << endl;
        return;
    }

    string pkg_group, pkg_name, pkg_ver, pkg_name_ver;

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(overlay_path))
    {
        if(entry.is_regular_file() and entry.path().extension() == ".ebuild" and entry.path().parent_path() != overlay_path)
        {
            pkg_name_ver = entry.path().stem();
            pkg_name = entry.path().parent_path().stem();
            pkg_group = entry.path().parent_path().parent_path().stem();

            if(pkg_groups.empty() or pkg_groups.back().get_name() != pkg_group)
            {
                pkg_groups.new_object(pkg_group);
            }

            pkg_groups.back().add_ebuild(pkg_name, pkg_name_ver);
        }
    }
}
