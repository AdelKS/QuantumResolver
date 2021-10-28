#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>

#include "ebuild.h"
#include "indexedvector.h"

class Package
{
public:
    Package(std::string pkg_group_name);

    Ebuild &add_version(const std::string &version);

    const string &get_pkg_name();
    Ebuild &get_ebuild(const string &ver);
    Ebuild &get_ebuild(const size_t &id);

    void update_useflags_with_constraints(const VersionConstraint &constraint, std::unordered_map<size_t, bool> useflag_states);

    void set_id(size_t pkg_id);

protected:
    string pkg_name; // e.g. sys-devel/gcc
    size_t pkg_id;

    IndexedVector<std::string, Ebuild> ebuilds; // indexed by ver, e.g. 11.1.0-r1
};

#endif // PACKAGE_H
