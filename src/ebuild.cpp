#include "ebuild.h"

using namespace std;

Ebuild::Ebuild(string ver): eversion(ver)
{

}

void Ebuild::add_useflag(size_t flag_id, bool default_state)
{
    useflags[flag_id] = default_state;
}

void Ebuild::add_useflags(std::unordered_map<std::size_t, bool> useflags_and_default_states)
{
    for(const auto &flag_state_iter: useflags_and_default_states)
    {
        add_useflag(flag_state_iter.first, flag_state_iter.second);
    }
}

void Ebuild::assign_useflag_state(size_t flag_id, bool state)
{
    auto flag_iter = useflags.find(flag_id);
    if(flag_iter != useflags.end())
        flag_iter->second = state;

}

void Ebuild::assign_useflag_states(std::unordered_map<std::size_t, bool> useflag_states)
{
    for(const auto &flag_state_iter: useflag_states)
    {
        assign_useflag_state(flag_state_iter.first, flag_state_iter.second);
    }
}

void Ebuild::set_pkg_id(size_t id)
{
    pkg_id = id;
}

size_t Ebuild::get_pkg_id()
{
    return pkg_id;
}

void Ebuild::set_id(size_t id)
{
    ebuild_id = id;
}

size_t Ebuild::get_ebuild_id()
{
    return ebuild_id;
}

bool Ebuild::operator <(const Ebuild &other)
{
    return eversion < other.eversion;
}

const EbuildVersion &Ebuild::get_version()
{
    return eversion;
}
