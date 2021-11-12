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

    Ebuild &add_version(const std::string &version, deque<string> &&ebuild_lines);

    void parse_metadata();
    void parse_deps();

    NamedVector<Ebuild>& get_ebuilds();

    const string &get_pkg_groupname();

    Ebuild& operator [](const size_t &id);
    Ebuild& operator [](const string &ver);

    void assign_useflag_states(const PackageConstraint &constraint,
                               const UseflagStates &useflag_states,
                               const FlagAssignType &assign_type = FlagAssignType::DIRECT);

    void set_id(size_t pkg_id);
    size_t get_id();

protected:
    std::string pkg_groupname; // e.g. sys-devel/gcc
    std::size_t pkg_id;

    std::shared_ptr<Parser> parser;
    NamedVector<Ebuild> ebuilds; // indexed by ver, e.g. 11.1.0-r1
};

#endif // PACKAGE_H
