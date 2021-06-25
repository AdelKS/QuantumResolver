#include "package.h"

Package::Package(int pkg_id, string pkg_name): id(pkg_id), name (pkg_name)
{

}

string Package::get_name()
{
    return name;
}

int Package::get_id()
{
    return id;
}

void Package::add_version(const string &version)
{
    ebuilds.new_object(version);
}
