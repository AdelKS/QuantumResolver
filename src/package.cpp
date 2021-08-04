#include <iostream>

#include "package.h"

using namespace std;

Package::Package(string pkg_group_name): group_name(pkg_group_name)
{

}

void Package::add_version(const string &version)
{
    ebuilds.new_object(version);
    if(ebuilds.size() > 5)
    {
        cout << "many ebuilds in this package!" << endl;
    }
}
