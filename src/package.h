#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>

#include "ebuild.h"
#include "indexedvector.h"

class Package
{
public:
    Package(std::string pkg_group_name);

    void add_version(const std::string &version);

protected:
    string group_name; // e.g. sys-devel/gcc

    IndexedVector<std::string, Ebuild, true> ebuilds; // indexed by ver, e.g. 11.1.0-r1

};

#endif // PACKAGE_H
