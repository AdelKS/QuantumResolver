#ifndef USEFLAGS_H
#define USEFLAGS_H

#include "named_vector.h"
#include "bijection.h"
#include "multikeymap.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <set>
#include <string_view>
#include <limits>

using ExpandID = std::size_t;
using FlagID = std::size_t;

enum struct FlagAssignType {DIRECT, MASK, STABLE_MASK, FORCE, STABLE_FORCE};

class Database;

enum struct FlagState {ON, OFF, FORCED, MASKED, UNKNOWN};

struct FlagGlobalInfo
{
    FlagGlobalInfo() : used(false), implicit(false), masked(false), forced(false) {};

    bool used, implicit, masked, forced;
    std::string expanded_from;
};


class UseFlags
{
public:

    UseFlags(Database *db);

    std::size_t add_flag(const std::string_view &flag_str);
    std::size_t get_flag_id(const std::string_view &flag_str);
    std::string get_flag_name(const size_t &id);

    void set_global_useflag(const std::string_view& flag_str);
    void unset_global_useflag(const std::string_view& flag_str);
    void unset_global_useflags();

    constexpr static std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    void populate_profile_flags();
    void clear_hidden_expands();
    void clear_normal_expands();
    void clear_implicit_expands();
    void clear_unprefixed_expands();
    void make_expand_hidden(std::size_t prefix_index, bool hidden);
    void make_expand_implicit(size_t prefix_index, bool implicit);
    void remove_expand(std::size_t prefix_index);

    void handle_use_line(const vector<string_view>& words);
    void handle_use_expand_line(string_view use_expand_type, const vector<string_view>& words);

    struct UseExpand
    {
        UseExpand(): unprefixed(false), implicit(false), hidden(false) {};
        UseExpand(string prefix): unprefixed(false), implicit(false), hidden(false), prefix(std::move(prefix)) {};

        bool unprefixed, implicit, hidden;
        string prefix;
    };

    MultiKeyMap<UseExpand, std::string, FlagID> use_expand;
    // use_expand characterization, reachable with the prefix string, and with the flag ids connected to the prefix

    Bijection<std::string, FlagID> useflags;
    // all the encountered useflags

    std::unordered_set<FlagID> use, use_mask, use_force, use_stable_force, use_stable_mask;
    // ids of the globally set useflags

    Database *db;
};

#endif // USEFLAGS_H