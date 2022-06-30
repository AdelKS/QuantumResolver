#ifndef EBUILD_H
#define EBUILD_H

#include <string>
#include <filesystem>
#include <limits>

#include "quantum-resolver/core/ebuild_version.h"
#include "quantum-resolver/utils/named_vector.h"
#include "quantum-resolver/core/parser.h"
#include "quantum-resolver/core/useflags.h"


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

    std::string get_slot_str() const;
    const std::string& get_slot() const;
    const std::string& get_subslot() const;

    bool respects_pkg_dep(const PackageDependency &pkg_dep);
    bool respects_pkg_constraint(const PackageConstraint &pkg_constraint);
    bool respects_usestates(const UseDependencies &use_dependencies);

    void accept_keywords(const Keywords& accept_these_keywords);
    void assign_useflag_state(std::size_t flag_id, bool state, const FlagAssignType &assign_type = FlagAssignType::DIRECT);
    void assign_useflag_states(const UseflagStates &useflag_states, const FlagAssignType &assign_type = FlagAssignType::DIRECT);

    void set_id(std::size_t id) { this->id = id; };
    void set_pkg_id(std::size_t id) { this->pkg_id = id; };

    std::size_t get_id() const { return id; };
    std::size_t get_pkg_id() const { return pkg_id; };
    Keywords::State get_arch_keyword() const;
    const EbuildVersion &get_version() const { return eversion; };

    bool is_keyword_accepted() const { return keyword_accepted; }

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

    std::filesystem::path ebuild_path, install_path;
    Keywords keywords;

    Dependencies bdeps, rdeps;
    std::unordered_map<std::string, std::string> ebuild_data;

    std::unordered_set<FlagID> iuse, iuse_effective;
    std::unordered_set<FlagID> use, use_mask, use_force;

    std::unordered_set<FlagID> install_time_active_flags;

    static const std::unordered_map<std::string, DependencyType> dependency_types;
    static const std::vector<std::string> metadata_vars;

    std::size_t id = npos, pkg_id = npos;
    std::string slot, subslot;

    bool masked = false, installed = false, changed_use = false;
    bool parsed_metadata = false, parsed_deps = false, finalized_flag_states = false;
    bool keyword_accepted = false;
};



#endif // EBUILD_H
