#ifndef PACKAGE_H
#define PACKAGE_H

#include <set>
#include <string>
#include <map>
#include <vector>

#include "ebuild.h"
#include "indexedvector.h"

using namespace std;

class Package
{
public:
    Package(string pkg_group_name);

    void add_version(const string &version);

protected:
    string group_name; // e.g. sys-devel/gcc

    IndexedVector<string, Ebuild, true> ebuilds; // indexed by ver, e.g. 11.1.0-r1

};

#endif // PACKAGE_H
