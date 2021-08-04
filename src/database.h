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
    IndexedVector<std::string, Package, false> pkgs;
    unordered_map<std::string, int> pkg_namever_to_id;
};

#endif // DATABASE_H
