#ifndef REPO_H
#define REPO_H

#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <limits>
#include <string>
#include <string_view>

#include "package.h"
#include "parser.h"
#include "string_utils.h"
#include "useflags.h"

class Database;

using PackageID = std::size_t;

class Repo
{
public:
    Repo(Database *db);

    bool is_system_pkg(PackageID pkg_id) const;
    bool is_selected_pkg(PackageID pkg_id) const;

    std::size_t get_pkg_id(const std::string_view &pkg_str) const;
    const std::string& get_pkg_groupname(std::size_t pkg_id) const;

    Package& operator [] (PackageID pkg_id) { return pkgs[pkg_id]; };

    constexpr static std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    void load_ebuilds(const std::string &path);
    void load_installed_pkgs();

    void parse_ebuild_metadata();
    void parse_deps();

    void load_package_accept_keywords();
    void load_package_useflag_settings();

    void load_system_packages();
    void load_selected_packages();

    NamedVector<Package> pkgs;
    std::unordered_set<PackageID> selected_pkgs, system_pkgs;

    Database *db;

};

#endif // REPO_H
