#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>
#include <filesystem>

#include "package.h"
#include "parser.h"
#include "misc_utils.h"

class Database
{
public:
    Database();

    std::size_t flag_id(const string_view &flag_str);
    void print_flag_states(const string &package);

    size_t get_useflag_id(const string_view &flag_str, bool create_ids);
    size_t get_pkg_id(const string_view &pkg_str);

    const string& get_pkg_groupname(size_t pkg_id);
    const string& get_useflag_name(size_t useflag_id);

    const std::size_t npos = -1;

    Parser parser;

protected:
    void load_ebuilds(const std::string &path);
    void load_profiles();
    void load_installed_pkgs();

    void parse_ebuild_metadata();
    void parse_deps();

    void load_masked_and_forced_useflags();

    void load_profiles_useflags(const std::vector<std::filesystem::path> &profile_tree);

    std::unordered_set<std::size_t> use_mask, use_force, use_stable_force, use_stable_mask;
    // global use flag overrides

    vector<PkgUseToggles> pkg_use, pkg_use_stable, pkg_use_mask, pkg_use_force, pkg_use_stable_force, pkg_use_stable_mask;
    // per pkg use flag overrides

    NamedVector<Package> pkgs;

    NamedVector<std::string> useflags;
    // use flag name to id

    NamedVector<std::size_t> use_expand_prefixes;
    // use-expand-prefixes are actually in useflags, we save her their ids

    std::unordered_map<std::size_t, std::size_t> use_expanded_flags;
    // flag ids that are use expanded, the linked id is the use-expand-prefix it is related to
};

#endif // DATABASE_H
