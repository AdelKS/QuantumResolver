#ifndef EBUILD_H
#define EBUILD_H

#include <string>

#include "ebuildversion.h"
#include "indexedvector.h"

class Dependencies
{
    struct DependencyNode
    {
        enum struct DependencyType {USEFLAG_COND, OR, PKG};
        DependencyType type;
        size_t id;
    };
};

class Ebuild
{

public:
    Ebuild(std::string ver);

    bool operator <(const Ebuild &other);

    void add_useflag(size_t flag_id, bool default_state);
    void add_useflags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    void assign_useflag_state(size_t flag_id, bool state);
    void assign_useflag_states(std::unordered_map<std::size_t, bool> useflag_states);

    void set_pkg_id(size_t id);
    size_t get_pkg_id();

    const EbuildVersion &get_version();

    void set_id(size_t id);
    size_t get_ebuild_id();

protected:
    std::unordered_map<size_t, bool> useflags;
    size_t pkg_id, ebuild_id;
    EbuildVersion eversion;
};



#endif // EBUILD_H
