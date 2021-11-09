#include <iostream>

#include "package.h"

using namespace std;
namespace fs = filesystem;

Package::Package(std::string pkg_group_name,
                 std::shared_ptr<Parser> parser):
    pkg_group_name(pkg_group_name), parser(parser)
{

}

const string &Package::get_pkg_name()
{
    return pkg_group_name;
}

void Package::set_id(size_t pkg_id)
{
    this->pkg_id = pkg_id;
}

size_t Package::get_id()
{
    return pkg_id;
}

void Package::parse_iuse()
{
    for(auto &ebuild: ebuilds)
    {
        ebuild.parse_iuse();
    }
}

Ebuild &Package::operator[](const string &ver)
{
    size_t index = ebuilds.id_of(ver);
    if(index == ebuilds.npos)
        throw runtime_error("version " + ver + "is not available in " + pkg_group_name);
    else return ebuilds[index];
}

Ebuild &Package::operator[](const size_t &id)
{
    return ebuilds[id];
}


Ebuild& Package::add_version(const string &version, const fs::path &path)
{
    size_t index = ebuilds.emplace_back(Ebuild(version, path, parser), version);
    ebuilds.back().set_id(index);
    ebuilds.back().set_pkg_id(pkg_id);

    return ebuilds[index];
}

void Package::update_useflags_with_constraints(const VersionConstraint &constraint,
                                               std::unordered_map<size_t, bool> useflag_states)
{
    for(auto &ebuild: ebuilds)
        if(respects_constraint(ebuild.get_version(), constraint))
            ebuild.assign_useflag_states(useflag_states);
}
