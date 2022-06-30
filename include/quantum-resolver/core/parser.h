#ifndef PARSER_H
#define PARSER_H

#include <type_traits>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <string>
#include <memory>
#include <limits>
#include <cassert>

#include "quantum-resolver/core/ebuild_version.h"
#include "quantum-resolver/utils/named_vector.h"
#include "quantum-resolver/utils/string_utils.h"

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

struct Keywords
{
    // The first column are the values that correspond to KEYWORDS
    // The second column are the values that correspond to ACCEPT_KEYWORDS
    enum struct State {BROKEN = 0, REFUSE = BROKEN,
                       STABLE, ACCEPT_STABLE = STABLE,
                       TESTING, ACCEPT_TESTING = TESTING,
                       LIVE,
                       UNDEFINED,
                                  ACCEPT_EVERYTHING};

    std::unordered_map<ArchID, State> explicitely_defined;
    State everything_else = State::UNDEFINED;

    State get_keyword(ArchID arch) const
    {
        auto it = explicitely_defined.find(arch);
        if(it != explicitely_defined.end())
            return it->second;
        else return everything_else;
    }

    bool respects(ArchID arch, const Keywords& accept_keywords) const
    {
        State arch_ebuild_keyword = get_keyword(arch);
        auto acccept_keyword = accept_keywords.get_keyword(arch);

        return arch_ebuild_keyword != Keywords::State::BROKEN and
                acccept_keyword != State::REFUSE and
                acccept_keyword >= arch_ebuild_keyword;
    }
};

class Database;

using UseflagStates = std::unordered_map<std::size_t, bool>;
using PkgUseToggles = std::pair<PackageConstraint, UseflagStates>;
using PkgAcceptkeywords = std::pair<PackageConstraint, Keywords>;

class Parser
{
public:

    Parser(Database *db);

    UseflagStates parse_useflags(const std::vector<std::string> &useflag_lines, bool default_state, bool create_flag_ids = false);
    UseflagStates parse_useflags(const std::string_view &useflags_str, bool default_state, bool create_ids = false);

    enum struct KeywordType {KEYWORDS, ACCEPT_KEYWORDS};
    Keywords parse_keywords(std::string_view keywords_str, KeywordType type);

    PkgUseToggles parse_pkguse_line(std::string_view pkg_useflag_toggles);
    PkgAcceptkeywords parse_pkg_accept_keywords_line(std::string_view pkg_accept_keywords_line);

    PackageDependency parse_pkg_dependency(std::string_view pkg_constraint_str);
    UseDependencies parse_pkg_usedeps(std::string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(std::string_view pkg_constraint_str);

protected:
    Database *db;

    enum struct SettingsType {USEFLAGS, ACCEPT_KEYWORDS};

    template <SettingsType type>
    auto parse_pkg_settings(std::string_view pkg_settings_line)
    {
        PackageConstraint pkg_constraint;

        // remove eventual spurious spaces
        skim_spaces_at_the_edges(pkg_settings_line);

        // find the first space that separates the package specification from the use flags
        // e.g.      >=app-misc/foo-1.2.3 +foo -bar
        //           first space here:   ^
        auto first_space_char = pkg_settings_line.find(' ');

        // early return in case of issues
        if(pkg_settings_line.empty() or
                pkg_settings_line.starts_with('#') or
                first_space_char == std::string_view::npos)
        {
            if constexpr (type == SettingsType::USEFLAGS)
                return std::make_pair(std::move(pkg_constraint), UseflagStates());
            else if constexpr (type == SettingsType::ACCEPT_KEYWORDS)
                return std::make_pair(std::move(pkg_constraint), Keywords());
        }

        // create a view on the package constraint str ">=app-misc/foo-1.2.3" then parse it
        std::string_view pkg_constraint_str_view(pkg_settings_line);
        pkg_constraint_str_view.remove_suffix(pkg_settings_line.size() - first_space_char);

        //pkg constraints cannot contain useflag constraints
        pkg_constraint = parse_pkg_constraint(pkg_constraint_str_view);

        // create a view on the useflags "+foo -bar" and parse it
        std::string_view settings_str_view(pkg_settings_line);
        settings_str_view.remove_prefix(pkg_constraint_str_view.size());

        if constexpr (type == SettingsType::USEFLAGS)
            return std::make_pair(std::move(pkg_constraint),
                             parse_useflags(settings_str_view, true, true));
        else if constexpr (type == SettingsType::ACCEPT_KEYWORDS)
            return std::make_pair(std::move(pkg_constraint),
                             parse_keywords(settings_str_view, KeywordType::ACCEPT_KEYWORDS));
        else throw std::runtime_error("Something is wrong here");
        // this last condition shouldn't be compiled in, as we should cover all
        // possibilites
    }

};

#endif // PARSER_H
