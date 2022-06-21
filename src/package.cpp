#include <iostream>
#include <algorithm>

#include "package.h"
#include "misc_utils.h"
#include "database.h"

using namespace std;
namespace fs = filesystem;

Package::Package(const std::string &pkg_group_name,
                 Database *db):
     pkg_groupname(pkg_group_name), db(db)
{
}

vector<size_t> Package::get_matching_ebuild_ids(const PackageConstraint &constraint)
{
    vector<size_t> matching_ebuilds;

    for(Ebuild &ebuild: ebuilds)
    {
        if(ebuild.respects_pkg_constraint(constraint))
            matching_ebuilds.push_back(ebuild.get_id());
    }

    std::ranges::sort(matching_ebuilds, [&](size_t ebuild_id_1, size_t ebuild_id_2)
                                        { return ebuilds[ebuild_id_1] < ebuilds[ebuild_id_2];});

    return matching_ebuilds;
}

const string &Package::get_pkg_groupname() const
{
    return pkg_groupname;
}

void Package::set_id(size_t pkg_id)
{
    this->pkg_id = pkg_id;
}

size_t Package::get_id() const
{
    return pkg_id;
}

std::size_t Package::size() const
{
    return ebuilds.size();
}

void Package::parse_metadata()
{
    for(auto &ebuild: ebuilds)
        ebuild.parse_metadata();
}

void Package::parse_deps()
{
    for(auto &ebuild: ebuilds)
        ebuild.parse_deps();
}

const NamedVector<Ebuild> &Package::get_ebuilds() const
{
    return ebuilds;
}

size_t Package::ebuild_id_of(const std::string &version)
{
    return ebuilds.index_of(version);
}

Ebuild &Package::operator[](const string &ver)
{
    size_t index = ebuilds.index_of(ver);
    if(index == ebuilds.npos)
        throw runtime_error("version " + ver + "is not available in " + pkg_groupname);
    else return ebuilds[index];
}

Ebuild &Package::operator[](EbuildID id)
{
    return ebuilds[id];
}

const Ebuild &Package::operator[](const string &ver) const
{
    size_t index = ebuilds.index_of(ver);
    if(index == ebuilds.npos)
        throw runtime_error("version " + ver + "is not available in " + pkg_groupname);
    else return ebuilds[index];
}

const Ebuild &Package::operator[](EbuildID id) const
{
    return ebuilds[id];
}

Ebuild& Package::add_version(const string &version)
{
    EbuildID ebuild_id = ebuild_id_of(version);
    if(ebuild_id == npos)
    {
        ebuild_id = ebuilds.push_back(Ebuild(version, db), version);
        ebuilds.back().set_id(ebuild_id);
        ebuilds.back().set_pkg_id(pkg_id);
    }

    return ebuilds[ebuild_id];
}

Ebuild& Package::add_repo_version(const string &version, const fs::path &ebuild_repo_path)
{
    Ebuild& ebuild = add_version(version);
    ebuild.set_ebuild_path(ebuild_repo_path);
    return ebuild;
}

Ebuild& Package::add_installed_version(const string& version, const std::filesystem::path &ebuild_install_path)
{
    Ebuild& ebuild = add_version(version);
    ebuild.set_install_path(ebuild_install_path);
    return ebuild;
}

void Package::assign_useflag_states(const PackageConstraint &constraint,
                                    const UseflagStates &useflag_states,
                                    const FlagAssignType &assign_type)
{
    if(constraint.pkg_id != pkg_id)
        throw runtime_error("Applying constraint to the wrong package");

    for(auto &ebuild: ebuilds)
        if(ebuild.respects_pkg_constraint(constraint))
            ebuild.assign_useflag_states(useflag_states, assign_type);
}

void Package::accept_keywords(const PackageConstraint &constraint,
                     const Keywords& accept_keywords)
{
    if(constraint.pkg_id != pkg_id)
        throw runtime_error("Applying constraint to the wrong package");

    for(auto &ebuild: ebuilds)
        if(ebuild.respects_pkg_constraint(constraint))
            ebuild.accept_keywords(accept_keywords);
}

Package::iterator Package::begin()
{
    return ebuilds.begin();
}

Package::iterator Package::end()
{
    return ebuilds.end();
}

Package::const_iterator Package::cbegin() const
{
    return ebuilds.cbegin();
}

Package::const_iterator Package::cend() const
{
    return ebuilds.cend();
}
