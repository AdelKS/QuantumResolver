#ifndef EBUILD_H
#define EBUILD_H

#include <string>
#include <filesystem>

#include "ebuildversion.h"
#include "namedvector.h"
#include "parser.h"

struct Dependencies
{
    enum struct Type {BUILD, RUNTIME};
    bool valid;
    std::vector<Dependencies> or_deps, xor_deps, at_most_one_deps, all_of_deps;
    std::vector<PackageConstraint> plain_deps;
    std::vector<std::pair<Toggle, Dependencies>> use_cond_deps;

};

typedef std::pair<size_t, size_t> EbuildId;

class Package;

class Ebuild
{

public:
    Ebuild(const std::string &ver,
           const std::filesystem::path &path,
           std::shared_ptr<Parser> parser);

    bool operator <(const Ebuild &other);
    void parse_dep_string();
    void parse_iuse();

    void assign_useflag_state(size_t flag_id, bool state);
    void assign_useflag_states(std::unordered_map<std::size_t, bool> useflag_states);

    void set_id(size_t id);
    size_t get_id();

    void set_pkg_id(size_t id);
    size_t get_pkg_id();

    const EbuildVersion &get_version();

    static const size_t npos = -1;

protected:
    Dependencies parse_dep_string(std::string_view dep_string);
    void add_deps(const Dependencies &deps, Dependencies::Type dep_type);

    void add_useflag(size_t flag_id, bool default_state);
    void add_useflags(std::unordered_map<std::size_t, bool> useflags_and_default_states);

    EbuildVersion eversion;
    std::filesystem::path ebuild_path;
    std::shared_ptr<Parser> parser;
    bool masked;

    Dependencies bdeps, rdeps;
    std::deque<std::string> ebuild_lines;
    std::unordered_map<size_t, bool> ebuild_useflags;
    size_t id, pkg_id;
};



#endif // EBUILD_H
