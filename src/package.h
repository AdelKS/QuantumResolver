#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <filesystem>

#include "ebuild.h"
#include "namedvector.h"
#include "parser.h"

class Package
{
public:
    Package(std::string pkg_group_name,
            std::shared_ptr<Parser> parser);

    Ebuild &add_version(const std::string &version, const std::filesystem::path &path);

    void parse_iuse();

    const string &get_pkg_name();

    Ebuild& operator [](const size_t &id);
    Ebuild& operator [](const string &ver);

    void update_useflags_with_constraints(const VersionConstraint &constraint, std::unordered_map<size_t, bool> useflag_states);

    void set_id(size_t pkg_id);
    size_t get_id();

protected:
    std::string pkg_group_name; // e.g. sys-devel/gcc
    std::size_t pkg_id;

    std::shared_ptr<Parser> parser;
    NamedVector<Ebuild> ebuilds; // indexed by ver, e.g. 11.1.0-r1
};

#endif // PACKAGE_H
