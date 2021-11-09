#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>

#include "package.h"
#include "parser.h"
#include "parseutils.h"

class Database
{
public:
    Database();

    void populate(const std::string &overlay_cache_path);

    static const size_t npos = -1;

protected:
    void load_ebuilds(const std::string &path);
    void parse_iuse();
    void parse_deps();

    void account_for_global_useflags();
    void account_for_user_useflags();

    void load_profile_settings();

    std::unordered_set<size_t> masked_flags, forced_flags;
    std::shared_ptr<NamedVector<Package>> pkgs;
    std::shared_ptr<NamedVector<std::string>> useflags;
    std::shared_ptr<Parser> parser;
};

#endif // DATABASE_H
