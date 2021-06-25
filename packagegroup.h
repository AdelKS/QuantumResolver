#ifndef PACKAGEGROUP_H
#define PACKAGEGROUP_H

#include <string>
#include <vector>
#include <map>

#include "package.h"
#include "stringindexedvector.h"

using namespace std;

class PackageGroup
{
public:
    PackageGroup(int group_id, string group_name);
    void add_ebuild(string pkg_name, string ebuild_stem);

    string get_name();
    int get_id();

protected:
    int id;
    string pkg_name;

    StringIndexedVector<Package> pkgs;
};

#endif // PACKAGEGROUP_H
