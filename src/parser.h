#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>

#include "ebuildversion.h"
#include "namedvector.h"

using UseflagStates = std::unordered_map<std::size_t, bool>;

struct SlotConstraint
{
    bool rebuild_on_slot_change, rebuild_on_subslot_change;
    std::string slot_str, subslot_str;
};

struct DirectUseDependency
{
    bool state;
    bool has_default_if_unexisting;
    bool default_if_unexisting;
};

struct ConditionalUseDependency
{
    bool forward_if_set, forward_if_not_set;
    bool forward_reverse_state;
};

struct UseDependency
{
    enum struct Type {DIRECT, CONDITIONAL};

    Type type;
    size_t id;

    DirectUseDependency direct_dep;
    ConditionalUseDependency cond_dep;
};

struct UseDependencies
{
    vector<UseDependency> use_deps;
    bool is_valid;
};

struct PackageConstraint
{
    bool is_valid;
    std::size_t pkg_id;
    VersionConstraint ver;
    SlotConstraint slot;
};

struct PackageDependency
{
    enum struct BlockerType {NONE, WEAK, STRONG};
    BlockerType blocker_type;
    bool is_valid;
    PackageConstraint pkg_constraint;
    UseDependencies use_dependencies;
};

using PkgUseToggles = std::pair<PackageConstraint, UseflagStates>;


struct Toggle
{
    size_t id;
    bool state;
};

class Package;

class Parser
{
public:
    Parser(std::shared_ptr<NamedVector<Package>> pkgs,
           std::shared_ptr<NamedVector<std::string>> useflags);    

    size_t useflag_id(const string_view &flag_str, bool create_ids);
    string pkg_groupname(size_t pkg_id);

    UseflagStates parse_useflags(const std::deque<std::string> &useflag_lines, bool default_state, bool create_flag_ids = false);
    UseflagStates parse_useflags(const std::string_view &useflags_str, bool default_state, bool create_ids = false);
    UseflagStates parse_keywords(const std::string_view &keywords_str);
    PkgUseToggles parse_pkguse_line(std::string_view pkg_useflag_toggles);

    PackageDependency parse_pkg_dependency(std::string_view pkg_constraint_str);
    UseDependencies parse_pkg_usedeps(std::string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(std::string_view pkg_constraint_str);

    static const size_t npos = -1;

protected:
    std::shared_ptr<NamedVector<Package>> pkgs;
    std::shared_ptr<NamedVector<std::string>> useflags;

};

#endif // PARSER_H
