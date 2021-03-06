#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <filesystem>
#include <deque>
#include <limits>

#include "quantum-resolver/core/parser.h"
#include "quantum-resolver/core/ebuild.h"

#include "quantum-resolver/utils/named_vector.h"

class Database;

using EbuildID = std::size_t;

class Package
{
public:
    typedef typename NamedVector<Ebuild>::iterator iterator;
    typedef typename NamedVector<Ebuild>::const_iterator const_iterator;

    Package(const std::string &pkg_group_name, Database *db);

    Ebuild& add_version(const std::string &version);

    Ebuild& add_repo_version(const std::string &version,
                             const std::filesystem::path &ebuild_repo_path);

    Ebuild& add_installed_version(const std::string& version,
                                  const std::filesystem::path &ebuild_install_path);

    EbuildID ebuild_id_of(const std::string &version);

    void parse_metadata();
    void parse_deps();

    const NamedVector<Ebuild>& get_ebuilds() const;

    const std::string& get_pkg_groupname() const;

    Ebuild& operator [](EbuildID id);
    Ebuild& operator [](const std::string &ver);

    const Ebuild& operator [](EbuildID id) const;
    const Ebuild& operator [](const std::string &ver) const;

    void assign_useflag_states(const PackageConstraint &constraint,
                               const UseflagStates &useflag_states,
                               const FlagAssignType &assign_type = FlagAssignType::DIRECT);

    void accept_keywords(const PackageConstraint &constraint, const Keywords& accept_keywords);

    std::vector<EbuildID> get_matching_ebuild_ids(const PackageConstraint &constraint);

    static constexpr EbuildID npos = NamedVector<Ebuild>::npos;

    void set_id(std::size_t pkg_id);
    std::size_t get_id() const;

    std::size_t size() const;
    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;

protected:

    std::string pkg_groupname; // e.g. sys-devel/gcc
    std::size_t pkg_id = npos;

    Database *db;
    NamedVector<Ebuild> ebuilds; // indexed by ver, e.g. 11.1.0-r1
};

#endif // PACKAGE_H
