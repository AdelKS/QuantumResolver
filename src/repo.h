#ifndef REPO_H
#define REPO_H

#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <limits>

#include "package.h"
#include "parser.h"
#include "misc_utils.h"
#include "useflags.h"

class Database;

class Repo
{
public:
    Repo(Database *db);

    void print_flag_states(const string &package);

    size_t get_useflag_id(const string_view &flag_str, bool create_ids);
    size_t get_pkg_id(const string_view &pkg_str);

    const string& get_pkg_groupname(size_t pkg_id);

    constexpr static std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    void load_ebuilds(const std::string &path);
    void load_installed_pkgs();

    void parse_ebuild_metadata();
    void parse_deps();

    void load_masked_and_forced_useflags();

    vector<PkgUseToggles> pkg_use, pkg_use_stable, pkg_use_mask, pkg_use_force, pkg_use_stable_force, pkg_use_stable_mask;
    // per pkg use flag overrides

    NamedVector<Package> pkgs;

    Database *db;

};

#endif // REPO_H
