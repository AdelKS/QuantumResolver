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

    Ebuild(std::string ver,
           Database *db);

    void set_ebuild_path(std::filesystem::path path);
    void set_install_path(std::filesystem::path path);

    bool operator <(const Ebuild &other);
    void parse_deps();
    void parse_metadata();

    FlagState get_flag_state(const std::size_t &flag_id) ;
    bool has_changed_use();
    bool is_installed() const;
    const std::unordered_set<FlagID>& get_iuse();
    const std::unordered_set<FlagID>& get_use();
    const std::unordered_set<FlagID>& get_use_mask();
    const std::unordered_set<FlagID>& get_use_force();

    std::unordered_set<FlagID> get_changed_flags();
    std::unordered_set<FlagID> get_enforced_flags();
    std::unordered_set<FlagID> get_active_flags();
    std::unordered_set<FlagID> get_install_active_flags();

    const std::string& get_slot() const;
    const std::string& get_subslot() const;

    bool respects_pkg_dep(const PackageDependency &pkg_dep);
    bool respects_pkg_constraint(const PackageConstraint &pkg_constraint);
    bool respects_usestates(const UseDependencies &use_dependencies);

    void assign_useflag_state(std::size_t flag_id, bool state, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void assign_useflag_states(const UseflagStates &useflag_states, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void set_id(std::size_t id);
    std::size_t get_id();

    void set_pkg_id(std::size_t id);
    std::size_t get_pkg_id();

    const EbuildVersion &get_version();

    static const std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:

    void load_install_time_active_flags();
    void finalize_flag_states();
    void load_data();

    Dependencies parse_dep_string(std::string_view dep_string);
    void add_deps(Dependencies deps, DependencyType dep_type);

    void add_iuse_flag(FlagID flag_id, bool default_state);
    void add_iuse_flags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    EbuildVersion eversion;
    Database* db;

    bool masked = false, installed = false, changed_use = false;
    bool parsed_metadata = false, parsed_deps = false, finalized_flag_states = false;
    std::filesystem::path ebuild_path, install_path;
    KeywordStates keywords;

    Dependencies bdeps, rdeps;
    std::unordered_map<std::string, std::string> ebuild_data;

    std::unordered_set<FlagID> iuse, iuse_effective;
    std::unordered_set<FlagID> use, use_mask, use_force;

    std::unordered_set<FlagID> install_time_active_flags;

    static const std::unordered_map<std::string, DependencyType> dependency_types;
    static const std::vector<std::string> metadata_vars;

    std::size_t id = npos, pkg_id = npos;
    std::string slot, subslot;
};



#endif // EBUILD_H
