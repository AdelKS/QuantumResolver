#include "packagegroup.h"

inline string get_version(const string &name_ver, const string &name)
{
    return name_ver.substr(name.size()+1, name_ver.size() - name.size()-1);
}

PackageGroup::PackageGroup(int group_id, string group_name): id (group_id), pkg_name (group_name)
{

}


void PackageGroup::add_ebuild(string pkg_name, string ebuild_stem)
{
    string pkg_ver = get_version(ebuild_stem, name);

    if(pkgs.back().get_name() == pkg_name)
    {
        pkgs.back().add_version(pkg_ver);
    }
    else
    {
        pkgs[pkg_name].add_version(pkg_ver);
    }
}

string PackageGroup::get_name()
{
    return pkg_name;
}

int PackageGroup::get_id()
{
    return id;
}
