#include <iostream>

#include "package.h"

using namespace std;

Package::Package(string pkg_group_name): pkg_name(pkg_group_name)
{

}

const string &Package::get_pkg_name()
{
    return pkg_name;
}

void Package::set_id(size_t pkg_id)
{
    this->pkg_id = pkg_id;
}

Ebuild &Package::get_ebuild(const string &ver)
{
    size_t index = ebuilds.index_of(ver);
    if(index == ebuilds.npos)
        throw runtime_error("version " + ver + "is not available in " + pkg_name);
    else return ebuilds[index];
}

Ebuild &Package::get_ebuild(const size_t &id)
{
    return ebuilds[id];
}


Ebuild& Package::add_version(const string &version)
{
    size_t index = ebuilds.new_object(version);
    ebuilds[index].set_pkg_id(pkg_id);

    return ebuilds[index];
}

void Package::update_useflags_with_constraints(const VersionConstraint &constraint,
                                               std::unordered_map<size_t, bool> useflag_states)
{
    for(auto &ebuild: ebuilds)
        if(respects_constraint(ebuild.get_version(), constraint))
            ebuild.assign_useflag_states(useflag_states);
}
