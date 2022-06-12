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
#include "misc_utils.h"
#include "useflags.h"

class Database;

using PackageID = std::size_t;

class Repo
{
public:
    Repo(Database *db);

    std::size_t get_pkg_id(const std::string_view &pkg_str);
    const std::string& get_pkg_groupname(std::size_t pkg_id);

    Package& operator [] (PackageID pkg_id);

    constexpr static std::size_t npos = std::numeric_limits<std::size_t>::max();

protected:
    void load_ebuilds(const std::string &path);
    void load_installed_pkgs();

    void parse_ebuild_metadata();
    void parse_deps();

    void load_package_useflag_settings();

    NamedVector<Package> pkgs;

    Database *db;

};

#endif // REPO_H
