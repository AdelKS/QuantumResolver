#ifndef USEFLAGS_H
#define USEFLAGS_H

#include "named_vector.h"
#include "bijection.h"
#include "multikey_map.h"
#include "misc_utils.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <set>
#include <string_view>
#include <limits>
#include <ranges>

using ExpandID = std::size_t;
using FlagID = std::size_t;
using FlagName = std::string;

enum struct FlagAssignType {DIRECT, MASK, STABLE_MASK, FORCE, STABLE_FORCE};

class Database;

enum struct FlagState {ON, OFF, FORCED, MASKED, NOT_IN_IUSE_EFFECTIVE};

struct FlagInfo
{
    bool used = false, masked = false, stable_masked = false, forced = false, stable_forced = false;
};

struct UseExpandName
{
    UseExpandName() {}
    explicit UseExpandName(std::string str) : name(to_upper(std::move(str))) {}
    explicit UseExpandName(const std::string_view& str_view) : name(to_upper(str_view)) {}
    explicit UseExpandName(const char* c_str) : name(to_upper(string_view(c_str))) {}

    bool operator == (const UseExpandName& other) const
    {
        return name == other.name;
    }

    std::string name;
};

namespace std {
    template <>
    struct hash<UseExpandName>
    {
        std::size_t operator()(const UseExpandName& k) const
        {
            return hash<string>{}(k.name);
        }
    };
}

struct UseExpandType
{
    bool unprefixed = false, implicit = false, hidden = false;
};

class UseFlags
{
public:

    UseFlags(Database *db);

    FlagID add_flag(const std::string_view &flag_str);
    FlagID get_flag_id(const std::string_view &flag_str) const;
    FlagName get_flag_name(const size_t &id) const;

    template <std::ranges::common_range Range> requires (std::is_same_v<std::ranges::range_value_t<Range>, size_t>)
    std::set<std::string> to_flag_names(const Range& flag_ids);

    std::pair<std::string, UseExpandType> get_use_expand_info(FlagID flag_id);

    FlagInfo get_flag_info(const std::string_view &flag_str) const;
    FlagInfo get_flag_info(FlagID id) const;

    FlagID get_arch_id() const;
    FlagName get_arch_name() const;

    const std::unordered_set<FlagID>& get_implicit_flags() const;
    const std::unordered_set<FlagID>& get_hidden_flags() const;
    const std::unordered_set<FlagID>& get_expand_flags() const;

    const std::unordered_set<FlagID>& get_use() const;

    const std::unordered_set<FlagID>& get_use_force() const;
    const std::unordered_set<FlagID>& get_use_stable_force() const;

    const std::unordered_set<FlagID>& get_use_mask() const;
    const std::unordered_set<FlagID>& get_use_stable_mask() const;

    constexpr static std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    void populate_profile_flags();

    void clear_globally_toggled_useflags();
    void clear_iuse_implicit_flags();
    void clear_hidden_expands();
    void clear_normal_expands();
    void clear_implicit_expands();
    void clear_unprefixed_expands();

    void set_arch();
    void make_expand_hidden(std::size_t prefix_index, bool hidden);
    void make_expand_implicit(std::size_t prefix_index, bool implicit);
    void remove_expand(std::size_t prefix_index);

    void handle_use_line(const vector<string_view>& flags);
    void handle_iuse_implicit_line(const vector<string_view> &flags);
    void handle_use_expand_line(string_view use_expand_type, const vector<string_view>& words);

    void compare_with_portage_eq();

    MultiKeyMap<UseExpandType, UseExpandName, FlagName, FlagID> use_expand;
    // use_expand characterization, reachable with the prefix string, and with the flag ids connected to the prefix

    Bijection<std::string, FlagID> useflags;
    // all the encountered useflags

    std::unordered_set<FlagID> implicit_useflags, hidden_useflags, expand_useflags;
    // all the implicit flags

    std::unordered_set<FlagID> use, use_mask, use_force, use_stable_force, use_stable_mask;
    // ids of the globally set useflags

    std::size_t current_arch = npos;
    std::string current_arch_name;
    Database *db;
};

#endif // USEFLAGS_H
