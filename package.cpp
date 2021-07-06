#include "package.h"

Package::Package(string pkg_group_name): group_name(pkg_group_name), ebuilds(true)
{

}

void Package::add_version(const string &version)
{
    ebuilds.new_object(version);
}

bool operator < (const Package &a, const Package &b)
{
    return true;
}
