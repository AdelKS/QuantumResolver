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

    void print_flag_states(const string &package);

protected:
    void load_installed_pkgs();
    void load_ebuilds(const std::string &path);
    void parse_ebuild_metadata();
    void parse_deps();

    void load_profile_settings();

    UseflagStates use_mask, use_force, use_stable_force, use_stable_mask;
    UseflagStates global_useflags;
    std::shared_ptr<NamedVector<Package>> pkgs;
    std::shared_ptr<NamedVector<std::string>> useflags;
    std::shared_ptr<Parser> parser;
};

#endif // DATABASE_H
