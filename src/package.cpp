#include <iostream>

#include "package.h"

using namespace std;

Package::Package(string pkg_group_name): group_name(pkg_group_name)
{

}

const string &Package::get_group_name()
{
    return group_name;
}

Ebuild &Package::get_ebuild(const string &group_namever)
{
    string ver = group_namever.substr(group_name.size()+1); // +1 to remove the leading dash, e.g. -11.1.0
    int index = ebuilds.index_of(ver);
    if(index == ebuilds.npos)
        throw "version " + ver + "is not available in " + group_name;
    else return ebuilds[index];
}


void Package::add_version(const string &version)
{
    ebuilds.new_object(version);
    if(ebuilds.size() > 5)
    {
        cout << "many ebuilds in this package!" << endl;
    }
}
