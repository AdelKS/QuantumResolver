#include <iostream>

#include "package.h"

using namespace std;
namespace fs = filesystem;

Package::Package(std::string pkg_group_name,
                 std::shared_ptr<Parser> parser):
    pkg_groupname(pkg_group_name), parser(parser)
{

}

const string &Package::get_pkg_groupname()
{
    return pkg_groupname;
}

void Package::set_id(size_t pkg_id)
{
    this->pkg_id = pkg_id;
}

size_t Package::get_id()
{
    return pkg_id;
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

NamedVector<Ebuild>& Package::get_ebuilds()
{
    return ebuilds;
}

Ebuild &Package::operator[](const string &ver)
{
    size_t index = ebuilds.id_of(ver);
    if(index == ebuilds.npos)
        throw runtime_error("version " + ver + "is not available in " + pkg_groupname);
    else return ebuilds[index];
}

Ebuild &Package::operator[](const size_t &id)
{
    return ebuilds[id];
}


Ebuild& Package::add_version(const string &version, deque<string> &&ebuild_lines)
{
    size_t index = ebuilds.emplace_back(Ebuild(version, move(ebuild_lines), parser), version);
    ebuilds.back().set_id(index);
    ebuilds.back().set_pkg_id(pkg_id);

    return ebuilds[index];
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
