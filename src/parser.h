#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>
#include <memory>

#include "ebuild_version.h"
#include "named_vector.h"



struct SlotConstraint
{
    SlotConstraint(): rebuild_on_slot_change(false), rebuild_on_subslot_change(false), slot_str(), subslot_str() {}

    bool rebuild_on_slot_change, rebuild_on_subslot_change;
    std::string slot_str, subslot_str;
};

struct DirectUseDependency
{
    DirectUseDependency(): state(false), has_default_if_unexisting(false), default_if_unexisting(false) {};

    bool state;
    bool has_default_if_unexisting;
    bool default_if_unexisting;
};

struct ConditionalUseDependency
{
    ConditionalUseDependency(): forward_if_set(false), forward_if_not_set(false), forward_reverse_state(false) {};

    bool forward_if_set, forward_if_not_set;
    bool forward_reverse_state;
};

struct UseDependency
{
    enum struct Type {DIRECT, CONDITIONAL};

    UseDependency(): type(Type::DIRECT), id(-1), direct_dep(), cond_dep() {};

    Type type;
    size_t id;

    DirectUseDependency direct_dep;
    ConditionalUseDependency cond_dep;
};

struct PackageConstraint
{
    PackageConstraint(): pkg_id(-1), ver(), slot() {};

    std::size_t pkg_id;
    VersionConstraint ver;
    SlotConstraint slot;
};

using UseDependencies = std::vector<UseDependency>;

struct PackageDependency
{
    enum struct BlockerType {NONE, WEAK, STRONG};

    PackageDependency(): blocker_type(BlockerType::NONE), pkg_constraint(), use_dependencies() {};

    BlockerType blocker_type;
    PackageConstraint pkg_constraint;
    UseDependencies use_dependencies;
};

struct Toggle
{
    Toggle(): id(-1), state(false) {};

    size_t id;
    bool state;
};

class Database;

using UseflagStates = std::unordered_map<std::size_t, bool>;
using PkgUseToggles = std::pair<PackageConstraint, UseflagStates>;

class Parser
{
public:

    Parser(Database *db);

    UseflagStates parse_useflags(const std::deque<std::string> &useflag_lines, bool default_state, bool create_flag_ids = false);
    UseflagStates parse_useflags(const std::string_view &useflags_str, bool default_state, bool create_ids = false);
    UseflagStates parse_keywords(const std::string_view &keywords_str);
    PkgUseToggles parse_pkguse_line(std::string_view pkg_useflag_toggles);

    PackageDependency parse_pkg_dependency(std::string_view pkg_constraint_str);
    UseDependencies parse_pkg_usedeps(std::string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(std::string_view pkg_constraint_str);

protected:
    Database *db;

};

#endif // PARSER_H
