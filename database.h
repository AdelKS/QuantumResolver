#ifndef DATABASE_H
#define DATABASE_H_H

#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

#include "package.h".h"
#include "utils.h"

using namespace std;

namespace fs = filesystem;

class Database
{
public:
    Database();

    void populate_from_cache_dir(string path);
    void populate_from_overlay(string path);

protected:
    StringIndexedVector<Package> pkgs;
};

#endif // DATABASE_H
