#include "ebuild.h"

#include "misc_utils.h"

#include "database.cpp"
#include "useflags.cpp"

using namespace std;

// TODO: separate CBUILD from BUILD, for cross compilation, in the future
const std::unordered_map<std::string, DependencyType> Ebuild::dependency_types =
{
    {"BDEPEND", DependencyType::BUILD},
    {"IDEPEND", DependencyType::BUILD},
    {"DEPEND", DependencyType::BUILD},
    {"RDEPEND", DependencyType::BUILD},
    {"PDEPEND", DependencyType::RUNTIME},
};
// DependencyType::RUNTIME here means its intuitive definition from the resolver point of view:
// "needed so the package can run", but not needed before (during the emerge).
// DependencyType::BUILD: needed before starting anything on the ebuild

Ebuild::Ebuild(string ver,
               std::filesystem::path ebuild_path,
               Database *db):
    eversion(std::move(ver)), db(db), masked(false), parsed_metadata(false), parsed_deps(false),
    ebuild_path(std::move(ebuild_path)), ebuild_type(EbuildType::UNKNOWN)
{
    if(eversion.is_live())
        ebuild_type = EbuildType::LIVE;
}

void Ebuild::parse_deps()
{
    if(parsed_deps)
        return;

    if(ebuild_lines.empty())
        ebuild_lines = read_file_lines(ebuild_path);

    for(string_view ebuild_line_view: ebuild_lines)
    {
        for(const auto& [dep_str, dep_type]: dependency_types)
        {
            if(ebuild_line_view.starts_with(dep_str))
            {
                ebuild_line_view.remove_prefix(dep_str.size() + 1);
                add_deps(parse_dep_string(ebuild_line_view), dep_type);
            }
        }
    }

    parsed_deps = true;
}

void Ebuild::parse_metadata()
{
    if(parsed_metadata)
        return;

    if(ebuild_lines.empty())
        ebuild_lines = read_file_lines(ebuild_path);

    for(string_view ebuild_line_view: ebuild_lines)
    {
        if(ebuild_line_view.starts_with("IUSE"))
        {
            // shrink by 5 to remove the "IUSE=" in the beginning
            ebuild_line_view.remove_prefix(5);

            auto flag_states = db->parser.parse_useflags(ebuild_line_view, false, true);
            add_iuse_flags(flag_states);

            unordered_set<string> use_names;
            for(size_t flag_id: use)
                use_names.insert(db->useflags.get_flag_name(flag_id));

        }
        else if(ebuild_line_view.starts_with("SLOT"))
        {
            ebuild_line_view.remove_prefix(5);
            size_t subslot_sep_index = ebuild_line_view.find_first_of("/");
            if(subslot_sep_index == string_view::npos)
            {
                slot = ebuild_line_view;
                subslot = ebuild_line_view;
            }
            else
            {
                slot = ebuild_line_view.substr(0, subslot_sep_index);
                subslot = ebuild_line_view.substr(subslot_sep_index+1);
            }
        }
        else if(ebuild_type != EbuildType::UNKNOWN and ebuild_line_view.starts_with("KEYWORDS"))
        {
            ebuild_line_view.remove_prefix(9);
            const auto &keywords = db->parser.parse_keywords(ebuild_line_view);
            for(const auto &[keyword_id, is_testing]: keywords)
            {
                if(db->useflags.get_arch_id() == keyword_id)
                {
                    ebuild_type = is_testing ? EbuildType::TESTING : EbuildType::STABLE;
                    break;
                }
            }
        }
    }

    iuse_effective = (db->useflags.get_implicit_flags() + iuse);

    use = (db->useflags.get_use() & iuse_effective) + use;
    use_force = db->useflags.get_use_force() & iuse_effective;
    use_mask = db->useflags.get_use_mask() & iuse_effective;

    if(ebuild_type == EbuildType::STABLE)
    {
        use_force = db->useflags.get_use_stable_force() & iuse_effective;
        use_mask = db->useflags.get_use_stable_mask() & iuse_effective;
    }

    parsed_metadata = true;
}

void Ebuild::add_iuse_flag(size_t flag_id, bool default_state)
{
    iuse.insert(flag_id);
    if(default_state)
        use.insert(flag_id);
}

void Ebuild::add_iuse_flags(std::unordered_map<std::size_t, bool> useflags_and_default_states)
{
    for(const auto &[flag_id, flag_state]: useflags_and_default_states)
    {
        add_iuse_flag(flag_id, flag_state);
    }
}

void Ebuild::assign_useflag_state(size_t flag_id, bool state, const FlagAssignType &assign_type)
{
    parse_metadata();

    if(not iuse_effective.contains(flag_id))
        return;

    if(assign_type == FlagAssignType::DIRECT)
    {
        if(state)
            use.insert(flag_id);
        else use.erase(flag_id);
    }
    else if(assign_type == FlagAssignType::FORCE or
            (assign_type == FlagAssignType::STABLE_FORCE and ebuild_type == EbuildType::STABLE))
    {
        if(state)
            use_force.insert(flag_id);
        else use_force.erase(flag_id);
    }
    else if(assign_type == FlagAssignType::MASK or
            (assign_type == FlagAssignType::STABLE_MASK and ebuild_type == EbuildType::STABLE))
    {
        if(state)
            use_mask.insert(flag_id);
        else use_mask.erase(flag_id);
    }
}

std::unordered_set<size_t> Ebuild::get_active_flags()
{
    if(not parsed_metadata)
        parse_metadata();

    return use + use_force - use_mask;
}

void Ebuild::assign_useflag_states(const UseflagStates &useflag_states, const FlagAssignType &assign_type)
{
    for(const auto &[flag_id, flag_state]: useflag_states)
    {
        assign_useflag_state(flag_id, flag_state, assign_type);
    }
}

void Ebuild::set_id(size_t id)
{
    this->id = id;
}

size_t Ebuild::get_id()
{
    return id;
}

void Ebuild::set_pkg_id(size_t id)
{
    pkg_id = id;
}

size_t Ebuild::get_pkg_id()
{
    return pkg_id;
}

bool Ebuild::operator <(const Ebuild &other)
{
    assert(pkg_id == other.pkg_id); // Make sure we are comparing ebuilds of the same package
    return eversion < other.eversion;
}

const EbuildVersion &Ebuild::get_version()
{
    return eversion;
}

void Ebuild::add_deps(Dependencies deps, DependencyType dep_type)
{
    Dependencies &m_deps = dep_type == DependencyType::BUILD ? bdeps : rdeps;

    // or_deps
    std::move(deps.or_deps.begin(), deps.or_deps.end(), std::back_inserter(m_deps.or_deps));

    // plain_deps
    std::move(deps.plain_deps.begin(), deps.plain_deps.end(), std::back_inserter(m_deps.plain_deps));

    // use_cond_deps
    std::move(deps.use_cond_deps.begin(), deps.use_cond_deps.end(), std::back_inserter(m_deps.use_cond_deps));

    //xor_deps
    std::move(deps.xor_deps.begin(), deps.xor_deps.end(), std::back_inserter(m_deps.xor_deps));
}

Dependencies Ebuild::parse_dep_string(string_view dep_string)
{
    /* Receives a dependency string as formatted in files in /var/db/repos/gentoo/metadata/md5-cache/
     * e.g. || ( dev-lang/python:3.10[ncurses,sqlite,ssl] dev-lang/python:3.9[ncurses,sqlite,ssl] dev-lang/python:3.8[ncurses,sqlite,ssl] ) app-arch/unzip
     */

    Dependencies deps;

    vector<tuple<string, std::vector<Dependencies>&>> pkg_group_dep_map
    {
        {"|| (", deps.or_deps},
        {"^^ (", deps.xor_deps},
        {"?? (", deps.at_most_one_deps},
        {"(", deps.all_of_deps}
    };

    skim_spaces_at_the_edges(dep_string);

    while(not dep_string.empty())
    {
        bool found_pkg_group;
        do
        {
            found_pkg_group = false;
            for(auto& [pkg_group_string, container]: pkg_group_dep_map)
            {
                if(dep_string.starts_with(pkg_group_string))
                {
                    dep_string.remove_prefix(pkg_group_string.size() - 1);

                    const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
                    Dependencies&& deps = parse_dep_string(enclosed_string);
                    if(not deps.valid)
                        return deps;

                    container.emplace_back(std::move(deps));

                    // move prefix to skip the enclosed content +2 to remove the parentheses
                    dep_string.remove_prefix(enclosed_string.size() + 2);

                    // so always the first character is not a space
                    skim_spaces_at_the_edges(dep_string);

                    found_pkg_group = true;
                }
            }
        }while(found_pkg_group);

        // it's okay if it returns npos
        size_t count = dep_string.find_first_of(' ');
        string_view constraint = dep_string.substr(0, count);
        dep_string.remove_prefix(constraint.size());

        if(constraint.ends_with('?'))
        {
            // This is flag condition

            if(count == string_view::npos)
                throw runtime_error("Use condition at the end of dep string: " + string(dep_string));

            constraint.remove_suffix(1);
            bool flag_state = true;
            if(constraint.starts_with('!'))
            {
                constraint.remove_prefix(1);
                flag_state = false;
            }

            FlagID flag_id = db->useflags.get_flag_id(constraint);
            if(flag_id == db->useflags.npos)
                return deps;

            // retrieve the enclosed content and add it to use_cond_deps

            skim_spaces_at_the_edges(dep_string);
            const string_view enclosed_string = get_pth_enclosed_string_view(dep_string);
            Dependencies&& use_cond_deps = parse_dep_string(enclosed_string);
            if(not use_cond_deps.valid)
                return deps;

            deps.use_cond_deps.emplace_back(flag_id, flag_state, std::move(use_cond_deps));

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else
        {
            // it is a "plain" (this may be a nested call) pkg constraint
            deps.plain_deps.push_back(db->parser.parse_pkg_dependency(constraint));
        }

        skim_spaces_at_the_edges(dep_string);
    }

    deps.valid = true;
    return deps;
}

void Ebuild::print_status()
{
    parse_metadata();

    unordered_set<string> use_names;
    for(size_t flag_id: use)
        use_names.insert(db->useflags.get_flag_name(flag_id));

    cout << "######################################" << endl;
    cout << db->repo.get_pkg_groupname(pkg_id) << "   Version: " << eversion.get_version() << endl;

    cout << "  USE=\"";
    for(const auto& flag_name: db->useflags.to_flag_names((use & iuse) - db->useflags.get_expand_flags()))
        cout << flag_name << " ";
    for(const auto& flag_name: db->useflags.to_flag_names(((iuse - use) - db->useflags.get_expand_flags())))
        cout << '-' << flag_name << " ";
    cout << "\"" << endl;

    auto enabled_expand_flag_ids = use & db->useflags.get_expand_flags();
    if(not enabled_expand_flag_ids.empty())
    {
        map<string, set<string>> enabled_expands;
        for(FlagID flag_id: enabled_expand_flag_ids)
        {
            string flag_name = db->useflags.get_flag_name(flag_id);
            auto [expand_name, expand_type] = db->useflags.get_use_expand_info(flag_id);
            if(not expand_type.hidden)
            {
                if(not expand_type.unprefixed)
                {
                    if(expand_name.size() + 2 >= flag_name.size())
                        throw runtime_error("Something went wrong");
                    flag_name = flag_name.substr(expand_name.size()+1);
                }
                enabled_expands[std::move(expand_name)].insert(std::move(flag_name));
            }

        }

        for(const auto& [expand_name, expand_flag_names]: enabled_expands)
        {
            cout << "  " << expand_name << "=\"";
            for(const auto& expand_flag_name: expand_flag_names)
                cout << expand_flag_name << " ";
            cout << "\"" << endl;
        }
    }
}

FlagState Ebuild::get_flag_state(const size_t &flag_id)
{
    if(not parsed_deps)
        parse_deps();

    FlagState state = FlagState::NOT_IN_IUSE_EFFECTIVE;
    if(not iuse_effective.contains(flag_id))
        return state;

    if(use_force.contains(flag_id))
        state = FlagState::FORCED;
    if(use_mask.contains(flag_id))
        state = FlagState::MASKED;

    if(state == FlagState::NOT_IN_IUSE_EFFECTIVE)
    {
        state = use.contains(flag_id) ? FlagState::ON : FlagState::OFF;
    }

    return state;
}

bool Ebuild::respects_usestates(const UseDependencies &use_dependencies)
{
    if(not parsed_deps)
        parse_deps();

    for(const UseflagDependency &use_dep: use_dependencies)
    {
        if(use_dep.type == UseflagDependency::Type::CONDITIONAL)
            throw runtime_error("Testing against conditional use dependencies, only direct is allowed");

        if(not iuse_effective.contains(use_dep.flag_id))
        {
            if(use_dep.direct_dep.has_default_if_unexisting and use_dep.direct_dep.state != use_dep.direct_dep.default_if_unexisting)
                return false;
            else throw runtime_error("In: " + db->repo.get_pkg_groupname(pkg_id) + "  id: " + to_string(id) + "\n" +
                                     "      Asking for useflag: " + db->useflags.get_flag_name(use_dep.flag_id) + " state but "
                                     " it has not been set and doesn't a fallback default");
        }
        else if(get_active_flags().contains(use_dep.flag_id) != use_dep.direct_dep.state)
            return false;
    }

    return true;
}

bool Ebuild::respects_pkg_constraint(const PackageConstraint &pkg_constraint)
{
    return (pkg_constraint.slot.slot_str.empty() or pkg_constraint.slot.slot_str == slot) and
                  (pkg_constraint.slot.subslot_str.empty() or pkg_constraint.slot.subslot_str == subslot) and
                  eversion.respects_constraint(pkg_constraint.ver);
}

bool Ebuild::respects_pkg_dep(const PackageDependency &pkg_dep)
{
    if(masked)
        return false;

    bool result = respects_pkg_constraint(pkg_dep.pkg_constraint) and respects_usestates(pkg_dep.use_dependencies);

    if(not (pkg_dep.blocker_type == PackageDependency::BlockerType::NONE))
        result = not result;

    return result;
}
