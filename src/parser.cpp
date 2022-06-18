#include "misc_utils.h"
#include "package.h"
#include "database.h"
#include "parser.h"
#include "src/useflags.h"

using namespace std;

Parser::Parser(Database *db) : db(db)
{}

UseflagStates Parser::parse_useflags(const vector<string> &useflag_lines, bool default_state, bool create_flag_ids)
{
    UseflagStates useflag_states;
    for(const string &string: useflag_lines)
    {
        for(const auto &[flag_id, state]: parse_useflags(string, default_state, create_flag_ids))
        {
            useflag_states[flag_id] = state;
        }
    }
    return useflag_states;
}

KeywordStates Parser::parse_keywords(string_view keywords_str, KeywordType type)
{
    /* Parses the keywords from a KEYWORDS="..." string
     * e.g. it can receive "~keyword1 keyword2 keyword3" as an input
     * returns a vector of Toggles, aka keyword_id (shares ids with useflags)
     * the bool is true when testing, i.e ~keyword
     * */

    static const unordered_map<char, KeywordStates::State> prefix_to_state =
    { {'-', KeywordStates::State::BROKEN}, {'~', KeywordStates::State::TESTING} };

    KeywordStates keyword_states;

    size_t word = 1;
    while(not keywords_str.empty())
    {
        if(keyword_states.everything_else == KeywordStates::State::LIVE)
            throw runtime_error("Settings keywords after ** is useless");

        KeywordStates::State state = KeywordStates::State::STABLE;
        string_view keyword = get_next_word(keywords_str); // get_next_word throws if the returned string_view is empty

        if(prefix_to_state.contains(keyword[0]))
        {
            state = prefix_to_state.at(keyword[0]);
            keyword.remove_prefix(1);
        }

        if(keyword == "*")
        {
            if(word != 1)
                throw runtime_error("* has been used in keywords but is not first");

            if (type == KeywordType::EBUILD and state != KeywordStates::State::BROKEN)
                throw runtime_error("* is not accepted in ebuilds");

            keyword_states.everything_else = state;
        }
        else if(keyword == "**")
        {
            if (type == KeywordType::EBUILD)
                throw runtime_error("** is not accepted in ebuilds");

            if(word != 1)
                throw runtime_error("* has been used in keywords but is not first");

            keyword_states.everything_else = KeywordStates::State::LIVE;
        }
        else
        {
            size_t arch_id = db->useflags.add_flag(keyword);
            if(not keyword_states.explicitely_defined.contains(arch_id) or
                    keyword_states.explicitely_defined[arch_id] < state)
                keyword_states.explicitely_defined[arch_id] = state;
        }

        word++;
    }

    return keyword_states;
}

UseflagStates Parser::parse_useflags(const string_view &useflags_str, bool default_state, bool create_flag_ids)
{
    /* Parses the use flag state toggles from a string
     * e.g. it can receive "-flag1 flag2 +flag3" as an input
     * default_state gives the state when it's simply 'flag' (not '+flag' nor '-flag')
     * returns a map between the flag id and its state, true if +flag or flag, false if -flag.
     * */

    auto start_it = useflags_str.begin(), end_it = useflags_str.begin();

    UseflagStates parsed_useflags;

    bool found_useflag = false;
    bool state = default_state;
    while (start_it != useflags_str.end())
    {
        if(not found_useflag)
        {
            if(*start_it == ' ')
            {
                start_it++;
                continue;
            }

            found_useflag = true;
            if(*start_it == '+')
            {
                state = true;
                start_it++;
            }
            else if(*start_it == '-')
            {
                state = false;
                start_it++;
            }

            end_it = start_it;
        }
        else
        {
            if(*end_it == ' ' or end_it == useflags_str.end())
            {
                size_t flag;
                if(create_flag_ids)
                    flag = db->useflags.add_flag(string_view(start_it, end_it));
                else flag = db->useflags.get_flag_id(string_view(start_it, end_it));

                if(flag == db->useflags.npos)
                    cout << "This useflag doesn't exist: " + string(string_view(start_it, end_it)) << endl;
                else parsed_useflags.insert(make_pair(flag, state));

                // reset the iterators and the state boolean
                start_it = end_it == useflags_str.end() ?  end_it : end_it + 1;
                state = default_state;
                found_useflag = false;
            }
            else end_it++;
        }
    }

    return parsed_useflags;
}

PkgUseToggles Parser::parse_pkguse_line(string_view pkguse_line)
{
    /* Parse a line that contains a pkg specification then flag toggles
     * e.g. ">=app-misc/foo-1.2.3:0 +flag1 -flag2 flag3"
     * */

    PackageConstraint pkg_constraint;
    UseflagStates flag_states;

    // remove eventual spurious spaces
    skim_spaces_at_the_edges(pkguse_line);

    if(pkguse_line.empty() or pkguse_line.starts_with('#'))
        return make_pair(pkg_constraint, flag_states);

    // find the first space that separates the package specification from the use flags
    // e.g.      >=app-misc/foo-1.2.3 +foo -bar
    //           first space here:   ^

    auto first_space_char = pkguse_line.find(' ');
    if(first_space_char == string_view::npos)
        return make_pair(pkg_constraint, flag_states);

    // create a view on the package constraint str ">=app-misc/foo-1.2.3" then parse it
    string_view pkg_constraint_str_view(pkguse_line);
    pkg_constraint_str_view.remove_suffix(pkguse_line.size() - first_space_char);

    //pkg constraints cannot contain useflag constraints
    pkg_constraint = parse_pkg_constraint(pkg_constraint_str_view);

    // create a view on the useflags "+foo -bar" and parse it
    string_view useflags_str_view(pkguse_line);
    useflags_str_view.remove_prefix(pkg_constraint_str_view.size());
    flag_states = parse_useflags(useflags_str_view, true, true);

    return make_pair(pkg_constraint, flag_states);
}

PackageDependency Parser::parse_pkg_dependency(string_view pkg_dep_str)
{
    string_view str(pkg_dep_str);

    PackageDependency pkg_dependency;
    skim_spaces_at_the_edges(str);

    // Check if there is useflag constraints
    string_view usedeps_str;
    if(str.ends_with(']'))
    {
        size_t start_index = str.find_first_of('[');
        if(start_index == string_view::npos or str.size() - start_index <= 2)
            throw runtime_error("Cannot parse package constraint " + string(pkg_dep_str));

        usedeps_str = str.substr(start_index + 1, str.size() - start_index - 2);

        pkg_dependency.use_dependencies = parse_pkg_usedeps(usedeps_str);

        str.remove_suffix(str.size() - start_index);
    }

    // check for blockers
    pkg_dependency.blocker_type = PackageDependency::BlockerType::NONE;
    if(str.starts_with("!!"))
    {
        str.remove_prefix(2);
        pkg_dependency.blocker_type = PackageDependency::BlockerType::STRONG;
    }
    else if(str.starts_with('!'))
    {
        str.remove_prefix(1);
        pkg_dependency.blocker_type = PackageDependency::BlockerType::WEAK;
    }

    pkg_dependency.pkg_constraint = parse_pkg_constraint(str);

    return pkg_dependency;
}

PackageConstraint Parser::parse_pkg_constraint(string_view pkg_constraint_str)
{
    /* Parses package constraint strings
     * e.g. "=app-misc/foo-1.2.3*:0=[-flag1(-),!flag2?,flag3]
     * allow_usedeps: allow having "[-flag1(-),!flag2?,flag3]" in the above example
     * */

    string_view str(pkg_constraint_str);

    // Reset slot constraints to defaults and check if there is slot constraints

    PackageConstraint pkg_constraint;

    pkg_constraint.slot.rebuild_on_slot_change = false;
    pkg_constraint.slot.rebuild_on_subslot_change = false;

    size_t slot_start = str.find_first_of(':');
    if(slot_start != string_view::npos)
    {
        string_view slot_constraint = str.substr(slot_start+1);
        if(slot_constraint.ends_with('='))
        {
            slot_constraint.remove_suffix(1);
            pkg_constraint.slot.rebuild_on_subslot_change = true;
            if(slot_constraint.empty())
                pkg_constraint.slot.rebuild_on_slot_change = true;
        }
        if(not slot_constraint.empty())
        {
            size_t subslot_sep_index = slot_constraint.find_first_of("/");
            if(subslot_sep_index == string_view::npos)
            {
                pkg_constraint.slot.slot_str = slot_constraint;
            }
            else
            {
                pkg_constraint.slot.slot_str = slot_constraint.substr(0, subslot_sep_index);
                pkg_constraint.slot.subslot_str = slot_constraint.substr(subslot_sep_index+1);
            }
        }

        str.remove_suffix(str.size() - slot_start);
    }

    if(str.ends_with('*') and not str.starts_with('='))
        throw runtime_error("Met weird package version constraint: " + string(pkg_constraint_str));

    string pkg_group_namever;
    string pkg_group_name;
    string pkg_ver;

    if(str.starts_with("<="))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::LESS;
        str.remove_prefix(2);
    }
    else if(str.starts_with(">="))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::GREATER;
        str.remove_prefix(2);
    }
    else if(str.starts_with('<'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SLESS;
        str.remove_prefix(1);
    }
    else if(str.starts_with('>'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SGREATER;
        str.remove_prefix(1);
    }
    else if(str.starts_with('~'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::EQ_REV;
        str.remove_prefix(1);
    }
    else if(str.starts_with('='))
    {
        str.remove_prefix(1);
        if(str.ends_with('*'))
        {
            pkg_constraint.ver.type = VersionConstraint::Type::EQ_STAR;
            str.remove_suffix(1);
        }
        else pkg_constraint.ver.type = VersionConstraint::Type::EQ;
    }
    else pkg_constraint.ver.type = VersionConstraint::Type::NONE;

    if(pkg_constraint.ver.type != VersionConstraint::Type::NONE)
    {
        size_t split_pos = pkg_namever_split_pos(str);
        pkg_group_name = str.substr(0, split_pos);
        pkg_ver = str.substr(split_pos+1);

        pkg_constraint.ver.version.set_version(pkg_ver);
    }
    else
    {
        // there is no version constraint so it's simply the package name
        pkg_group_name = str;
    }

    pkg_constraint.pkg_id = db->repo.get_pkg_id(pkg_group_name);

    return pkg_constraint;
}

UseDependencies Parser::parse_pkg_usedeps(string_view useflags_constraint_str)
{
    /* Parses the use flag constraint string inside a pkg dependency specification
     * e.g. it can receive "-flag1(-),!flag2?,flag3" as an input
     *      from ">=app-misc/foo-1.2.3:0=[-flag1(-),!flag2?,flag3]
     * */

    UseDependencies use_dependencies;

    size_t next_comma;
    string_view single_constraint;

    bool done = false;
    while(not done)
    {
        UseflagDependency usedep;
        next_comma = useflags_constraint_str.find_first_of(',');

        if (next_comma != string_view::npos)
        {
            single_constraint = useflags_constraint_str.substr(0, next_comma);
            useflags_constraint_str.remove_prefix(next_comma + 1);
        }
        else
        {
            single_constraint = useflags_constraint_str;
            done = true;
        }

        if(single_constraint.starts_with('!'))
        {
            single_constraint.remove_prefix(1);
            usedep.type = UseflagDependency::Type::CONDITIONAL;

            if(single_constraint.ends_with('?'))
            {
                single_constraint.remove_suffix(1);
                usedep.cond_dep.forward_if_not_set = true;
                usedep.cond_dep.forward_if_set = false;
                usedep.cond_dep.forward_reverse_state = false;
            }
            else if(single_constraint.ends_with('='))
            {
                single_constraint.remove_suffix(1);
                usedep.cond_dep.forward_if_not_set = true;
                usedep.cond_dep.forward_if_set = true;
                usedep.cond_dep.forward_reverse_state = true;
            }
            else throw runtime_error("Could not prase use flag constraint string: " + string(useflags_constraint_str));
        }
        else if(single_constraint.ends_with('?'))
        {
            usedep.type = UseflagDependency::Type::CONDITIONAL;
            single_constraint.remove_suffix(1);
            usedep.cond_dep.forward_if_not_set = false;
            usedep.cond_dep.forward_if_set = true;
            usedep.cond_dep.forward_reverse_state = false;
        }
        else if(single_constraint.ends_with('='))
        {
            usedep.type = UseflagDependency::Type::CONDITIONAL;
            single_constraint.remove_suffix(1);
            usedep.cond_dep.forward_if_not_set = true;
            usedep.cond_dep.forward_if_set = true;
            usedep.cond_dep.forward_reverse_state = false;
        }
        else
        {
            usedep.type = UseflagDependency::Type::DIRECT;

            usedep.direct_dep.state = true;
            if(single_constraint.starts_with('-'))
            {
                usedep.direct_dep.state = false;
                single_constraint.remove_prefix(1);
            }
            else if(single_constraint.starts_with('+'))
                single_constraint.remove_prefix(1);
        }

        usedep.direct_dep.default_if_unexisting = false;
        if(single_constraint.ends_with("(+)"))
        {
            usedep.direct_dep.default_if_unexisting = true;
            single_constraint.remove_suffix(3);
        }
        if(single_constraint.ends_with("(-)"))
        {
            usedep.direct_dep.default_if_unexisting = true;
            single_constraint.remove_suffix(3);
        }

        usedep.flag_id = db->useflags.get_flag_id(single_constraint);
        use_dependencies.emplace_back(move(usedep));
    }

    return use_dependencies;
}
