#ifndef EBUILD_H
#define EBUILD_H

#include <string>
#include <filesystem>

#include "ebuildversion.h"
#include "namedvector.h"
#include "parser.h"

enum struct FlagAssignType {DIRECT, MASK, STABLE_MASK, FORCE, STABLE_FORCE};

struct Dependencies
{
    enum struct Type {BUILD, RUNTIME};
    bool valid;
    std::vector<Dependencies> or_deps, xor_deps, at_most_one_deps, all_of_deps;
    std::vector<PackageDependency> plain_deps;
    std::vector<std::pair<Toggle, Dependencies>> use_cond_deps;
};

class Package;

class Ebuild
{
public:
    enum struct VersionType {STABLE, TESTING, LIVE};
    enum struct FlagState {ON, OFF, FORCED, MASKED, UNKNOWN};

    Ebuild(const std::string &ver,
           const std::filesystem::path &ebuild_path,
           shared_ptr<Parser> &parser);

    bool operator <(const Ebuild &other);
    void parse_deps();
    void parse_metadata();

    void print_flag_states(bool iuse_only = true);

    FlagState get_flag_state(const size_t &flag_id);
    bool respects_pkg_dep(const PackageDependency &pkg_dep);

    bool respects_pkg_constraint(const PackageConstraint &pkg_constraint);
    bool respects_usestates(const UseDependencies &use_dependencies);

    void assign_useflag_state(size_t flag_id, bool state, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void assign_useflag_states(const UseflagStates &useflag_states, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void set_id(size_t id);
    size_t get_id();

    void set_pkg_id(size_t id);
    size_t get_pkg_id();

    const EbuildVersion &get_version();

    static const size_t npos = -1;

protected:

    Dependencies parse_dep_string(std::string_view dep_string);
    void add_deps(const Dependencies &deps, Dependencies::Type dep_type);
    void add_deps(const Dependencies &&deps, Dependencies::Type dep_type);

    void add_iuse_flag(size_t flag_id, bool default_state);
    void add_iuse_flags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    EbuildVersion eversion;
    std::shared_ptr<Parser> parser;

    bool masked, parsed_metadata, parsed_deps;
    std::filesystem::path ebuild_path;
    VersionType version_type;

    Dependencies bdeps, rdeps;
    std::deque<std::string> ebuild_lines;

    std::unordered_set<size_t> iuse_flags;
    std::unordered_set<size_t> masked_flags, forced_flags;
    UseflagStates flag_states;

    size_t id, pkg_id;
    std::string slot, subslot;
};



#endif // EBUILD_H
