#ifndef EBUILD_H
#define EBUILD_H

#include <string>

#include "ebuildversion.h"
#include "indexedvector.h"

struct SlotConstraint
{
    bool rebuild_on_slot_change, rebuild_on_subslot_change;
    std::string slot_str, subslot_str;
};

struct UseflagConstraint
{
    enum struct Type {DIRECT, CONDITIONAL};

    Type type;
    size_t id;
    bool state;
    bool default_if_unexisting; // if the target package does not offer the use flag

    bool forward_if_set, forward_if_not_set;
    bool forward_reverse_state;
};

struct PackageConstraint
{
    enum struct BlockerType {NONE, WEAK, STRONG};
    BlockerType blocker_type;
    std::size_t pkg_id;
    VersionConstraint ver;
    SlotConstraint slot;
    std::vector<UseflagConstraint> flags;
};

struct UseflagCondition
{
    size_t flag_id;
    bool state;
};

struct Dependencies
{
    enum struct Type {BUILD, RUNTIME};
    std::vector<Dependencies> or_deps;
    std::vector<std::pair<UseflagCondition, Dependencies>> use_cond_deps;
    std::vector<PackageConstraint> plain_deps;
};

class Package;

class Ebuild
{

public:
    Ebuild(std::string ver);

    bool operator <(const Ebuild &other);

    void add_deps(const Dependencies &deps, Dependencies::Type dep_type);

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
    Dependencies bdeps, rdeps;

    std::unordered_map<size_t, bool> ebuild_useflags;
    size_t pkg_id, ebuild_id;
    EbuildVersion eversion;
};



#endif // EBUILD_H
