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

typedef std::pair<size_t, size_t> EbuildId;

class Package;

class Ebuild
{

public:
    Ebuild(const std::string &ver,
           std::deque<string> &&ebuild_lines,
           std::shared_ptr<Parser> parser);

    bool operator <(const Ebuild &other);
    void parse_deps();
    void parse_metadata();

    bool respects_pkg_dep(const PackageDependency &pkg_dep);

    bool respects_pkg_constraint(const PackageConstraint &pkg_constraint);
    bool respects_usestates(const UseDependencies &use_dependencies);

    void assign_useflag_state(size_t flag_id, bool state, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void assign_useflag_states(UseflagStates useflag_states, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void set_id(size_t id);
    size_t get_id();

    void set_pkg_id(size_t id);
    size_t get_pkg_id();

    const EbuildVersion &get_version();

    static const size_t npos = -1;

protected:
    Dependencies parse_dep_string(std::string_view dep_string);
    void add_deps(const Dependencies &deps, Dependencies::Type dep_type);

    void add_useflag(size_t flag_id, bool default_state);
    void add_useflags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    EbuildVersion eversion;
    std::shared_ptr<Parser> parser;
    bool masked, testing;

    Dependencies bdeps, rdeps;
    std::deque<std::string> ebuild_lines;

    std::unordered_set<size_t> masked_flags, forced_flags;
    UseflagStates ebuild_useflags;
    UseflagStates global_useflags;
    size_t id, pkg_id;
    std::string slot, subslot;
};



#endif // EBUILD_H
