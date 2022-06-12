#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>
#include <memory>
#include <limits>

#include "ebuild_version.h"
#include "named_vector.h"

typedef std::size_t ExpandID;
typedef std::size_t FlagID;
typedef std::size_t ArchID;
typedef std::string FlagName;

struct SlotConstraint
{
    bool rebuild_on_slot_change = false, rebuild_on_subslot_change = false;
    std::string slot_str, subslot_str;
};

struct DirectUseDependency
{
    bool state = false;
    bool has_default_if_unexisting = false;
    bool default_if_unexisting = false;
};

struct ConditionalUseDependency
{
    bool forward_if_set = false;
    bool forward_if_not_set = false;
    bool forward_reverse_state = false;
};

struct UseflagDependency
{
    enum struct Type {DIRECT, CONDITIONAL};

    Type type = Type::DIRECT;
    size_t flag_id = std::numeric_limits<size_t>::max();

    DirectUseDependency direct_dep;
    ConditionalUseDependency cond_dep;
};

struct PackageConstraint
{
    std::size_t pkg_id = std::numeric_limits<size_t>::max();
    VersionConstraint ver;
    SlotConstraint slot;
};

using UseDependencies = std::vector<UseflagDependency>;

struct PackageDependency
{
    enum struct BlockerType {NONE, WEAK, STRONG};

    BlockerType blocker_type = BlockerType::NONE;
    PackageConstraint pkg_constraint;
    UseDependencies use_dependencies;
};

struct Toggle
{
    size_t id = std::numeric_limits<size_t>::max();
    bool state = false;
};

struct KeywordStates
{
    enum struct State {UNDEFINED, BROKEN, STABLE, TESTING, LIVE};
    std::unordered_map<ArchID, State> explicitely_defined;
    State everything_else = State::UNDEFINED;

    State get_state(ArchID arch)
    {
        auto it = explicitely_defined.find(arch);
        if(it != explicitely_defined.end())
            return it->second;
        else return everything_else;
    }
};

class Database;

using UseflagStates = std::unordered_map<std::size_t, bool>;
using PkgUseToggles = std::pair<PackageConstraint, UseflagStates>;

class Parser
{
public:

    Parser(Database *db);

    UseflagStates parse_useflags(const std::vector<std::string> &useflag_lines, bool default_state, bool create_flag_ids = false);
    UseflagStates parse_useflags(const std::string_view &useflags_str, bool default_state, bool create_ids = false);

    enum struct KeywordType {USER, EBUILD};
    KeywordStates parse_keywords(std::string_view keywords_str, KeywordType type);

    PkgUseToggles parse_pkguse_line(std::string_view pkg_useflag_toggles);

    PackageDependency parse_pkg_dependency(std::string_view pkg_constraint_str);
    UseDependencies parse_pkg_usedeps(std::string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(std::string_view pkg_constraint_str);

protected:


    Database *db;

};

#endif // PARSER_H
