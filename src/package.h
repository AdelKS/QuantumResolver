#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <filesystem>
#include <deque>

#include "ebuild.h"
#include "named_vector.h"
#include "parser.h"

class Database;

class Package
{
public:
    Package(const std::string &pkg_group_name, Database *db);

    Ebuild &add_version(const std::string &version, const std::filesystem::path &ebuild_path);

    std::size_t id_of(const std::string &version);

    void parse_metadata();
    void parse_deps();

    NamedVector<Ebuild>& get_ebuilds();

    const string &get_pkg_groupname();

    Ebuild& operator [](const size_t &id);
    Ebuild& operator [](const string &ver);

    void assign_useflag_states(const PackageConstraint &constraint,
                               const UseflagStates &useflag_states,
                               const FlagAssignType &assign_type = FlagAssignType::DIRECT);

    const std::vector<std::size_t> get_matching_ebuilds(const PackageConstraint &constraint);

    void set_installed_version(const std::string &version, const std::string &activated_useflags);

    void set_id(size_t pkg_id);
    size_t get_id();

protected:

    struct InstalledPkg
    {
        InstalledPkg() : ebuild_id(-1), activated_useflags() {}
        std::size_t ebuild_id; // can be npos
        std::unordered_set<std::size_t> activated_useflags;
    };

    std::string pkg_groupname; // e.g. sys-devel/gcc
    std::size_t pkg_id;

    Database *db;
    NamedVector<Ebuild> ebuilds; // indexed by ver, e.g. 11.1.0-r1
    InstalledPkg installed_pkg;
};

#endif // PACKAGE_H
