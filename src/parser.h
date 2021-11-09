#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>

#include "ebuildversion.h"
#include "namedvector.h"

struct SlotConstraint
{
    bool rebuild_on_slot_change, rebuild_on_subslot_change;
    std::string slot_str, subslot_str;
};

struct UseflagConstraint
{
    enum struct Type {DIRECT, CONDITIONAL};
    bool is_valid;

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
    bool is_valid;
    BlockerType blocker_type;
    std::size_t pkg_id;
    VersionConstraint ver;
    SlotConstraint slot;
    std::vector<UseflagConstraint> flags;
};

struct Toggle
{
    size_t id;
    bool state;
};


using PkgUseToggles = std::pair<PackageConstraint, std::unordered_map<size_t, bool>>;
using UseflagStates = std::unordered_map<std::size_t, bool>;

class Package;

class Parser
{
public:
    Parser(std::shared_ptr<NamedVector<Package>> pkgs,
           std::shared_ptr<NamedVector<std::string>> useflags);    

    size_t useflag_id(const string_view &flag_str, bool create_ids);

    UseflagStates parse_useflags(const std::string_view &useflags_str, bool default_state, bool create_ids = false);
    PkgUseToggles parse_pkguse_line(std::string_view pkg_useflag_toggles);
    std::vector<UseflagConstraint> parse_pkg_useflag_constraints(std::string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(std::string_view pkg_constraint_str, bool permit_useflags=true);

    static const size_t npos = -1;

protected:
    std::shared_ptr<NamedVector<Package>> pkgs;
    std::shared_ptr<NamedVector<std::string>> useflags;

};

#endif // PARSER_H
