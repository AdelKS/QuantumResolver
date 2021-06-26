#ifndef PACKAGE_H
#define PACKAGE_H

#include <set>
#include <string>
#include <map>
#include <vector>

#include "ebuild.h"
#include "stringindexedvector.h"

using namespace std;

class Package
{
public:
    Package(int pkg_id, string pkg_name);

    void add_version(const string &version);

    string get_name();
    int get_id();

protected:
    int id;
    string name; // e.g. sys-devel/gcc

    StringIndexedVector<Ebuild> ebuilds; // indexed by name_ver, e.g. gcc-11.1.0-r1

};

#endif // PACKAGE_H
