#ifndef DATABASE_H
#define DATABASE_H_H

#include <unordered_map>
#include <string>

#include "package.h"
#include "utils.h"

class Database
{
public:
    Database();

    void populate_from_cache_dir(std::string path);
    void populate_from_overlay(std::string path);

protected:
    void add_use_flags_to_ebuild(const string &uses_string, Package &pkg, const string &pkg_group_namever);

    IndexedVector<std::string, Package, false> pkgs;
    unordered_map<std::string, int> pkg_group_namever_to_pkg_id;
    unordered_map<std::string, int> iuse_to_id;
};

#endif // DATABASE_H
