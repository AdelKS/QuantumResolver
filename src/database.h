#ifndef DATABASE_H
#define DATABASE_H_H

#include <unordered_map>
#include <string>

#include "package.h"
#include "utils.h"

class Database
{
    using EbuildId = std::pair<size_t, size_t> ;

public:
    Database();

    void populate_from_cache_dir(std::string path);

    static const size_t npos = -1;

protected:
    std::unordered_map<std::size_t, bool> parse_useflags(const string_view &useflags_str, bool default_state, bool create_ids = false);
    std::vector<UseflagConstraint> parse_useflag_constraints(string_view useflags_constraint_str);
    PackageConstraint parse_pkg_constraint(string_view pkg_constraint_str);
    std::size_t get_useflag_id(const string &useflag_str, bool create_ids = false);

    Dependencies parse_dependencies(string_view dep_string);

    void account_for_global_useflags();
    void account_for_user_useflags();

    IndexedVector<std::string, Package> pkgs;
    std::unordered_map<std::string, std::size_t> useflag_str_to_id;
};

#endif // DATABASE_H
