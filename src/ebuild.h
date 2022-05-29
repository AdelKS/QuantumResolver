#ifndef EBUILD_H
#define EBUILD_H

#include <string>
#include <filesystem>
#include <limits>

#include "ebuild_version.h"
#include "named_vector.h"
#include "parser.h"
#include "useflags.h"


enum struct DependencyType {BUILD, RUNTIME};

struct Dependencies
{
    bool valid = false;
    std::vector<Dependencies> or_deps, xor_deps, at_most_one_deps, all_of_deps;
    std::vector<PackageDependency> plain_deps;
    std::vector<std::tuple<FlagID, bool, Dependencies>> use_cond_deps;
};

class Database;

class Ebuild
{
public:
    enum struct EbuildType {UNKNOWN, STABLE, TESTING, LIVE};

    Ebuild(std::string ver,
           std::filesystem::path ebuild_path,
           Database *db);

    bool operator <(const Ebuild &other);
    void parse_deps();
    void parse_metadata();

    void print_status();

    FlagState get_flag_state(const size_t &flag_id);
    std::unordered_set<size_t> get_active_flags();

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

    static const size_t npos = std::numeric_limits<std::size_t>::max();

protected:

    Dependencies parse_dep_string(std::string_view dep_string);
    void add_deps(Dependencies deps, DependencyType dep_type);

    void add_iuse_flag(size_t flag_id, bool default_state);
    void add_iuse_flags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    EbuildVersion eversion;
    Database* db;

    bool masked, parsed_metadata, parsed_deps;
    std::filesystem::path ebuild_path;
    EbuildType ebuild_type;

    Dependencies bdeps, rdeps;
    std::deque<std::string> ebuild_lines;

    std::unordered_set<FlagID> iuse, iuse_effective;
    std::unordered_set<FlagID> use, use_mask, use_force;

    static const std::unordered_map<std::string, DependencyType> dependency_types;

    size_t id, pkg_id;
    std::string slot, subslot;
};



#endif // EBUILD_H
