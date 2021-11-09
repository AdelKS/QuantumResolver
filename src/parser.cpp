#include "parser.h"

#include "parseutils.h"

using namespace std;

Parser::Parser(shared_ptr<NamedVector<Package> > pkgs,
               shared_ptr<NamedVector<string> > useflags) :
    pkgs(pkgs), useflags(useflags)
{

}

size_t Parser::useflag_id(const string_view &flag_str, bool create_ids)
{
    /* Returns the id of the useflag given by useflag_str
     * if create_ids = true, it creates an id if it doesn't exist already
     * if create_ids = false and the string hasn't been encountered before, returns npos
     */

    // TODO: figure out how to use string_view for performance
    //       c.f. https://en.cppreference.com/w/cpp/container/unordered_map/find

    size_t useflag_id = useflags->id_of(flag_str);
    if(useflag_id == npos)
    {
        if(create_ids)
        {
            useflag_id = useflags->emplace_back(string(flag_str), flag_str);
        }
    }

    return useflag_id;
}

UseflagStates Parser::parse_useflags(const string_view &useflags_str, bool default_state, bool create_flag_ids)
{
    /* Parses the use flag state toggles from a string
     * e.g. it can receive "-flag1 flag2 +flag3" as an input
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
                // We isolated a useflag between start_it and end_it
                // TODO: string(string_view()) is counter-productive for performance
                //       figure out how to do it with Hash::is_transparent and KeyEqual::is_transparent
                //       c.f. https://en.cppreference.com/w/cpp/container/unordered_map/find
                size_t flag = useflag_id(string_view(start_it, end_it), create_flag_ids);
                if(flag == npos)
                    cout << "This useflag doesn't exist: " + string(string_view(start_it, end_it)) << endl;

                // add flag id and its state to the list
                parsed_useflags.insert(make_pair(flag, state));

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


pair<PackageConstraint, unordered_map<size_t, bool>> Parser::parse_pkguse_line(string_view pkguse_line)
{
    /* Parse a line that contains a pkg specification then flag toggles
     * e.g. ">=app-misc/foo-1.2.3:0 +flag1 -flag2 flag3"
     * */

    pair<PackageConstraint, unordered_map<size_t, bool>> result;
    result.first.is_valid = false;

    // remove eventual spurious spaces
    skim_spaces_at_the_edges(pkguse_line);

    if(pkguse_line.empty() or pkguse_line.starts_with('#'))
        return result;

    // find the first space that separates the package specification from the use flags
    // e.g.      >=app-misc/foo-1.2.3 +foo -bar
    //           first space here:   ^

    size_t first_space_char = pkguse_line.find(' ');
    if(first_space_char == string_view::npos)
        return result;

    // create a view on the package constraint str ">=app-misc/foo-1.2.3" then parse it
    string_view pkg_constraint_str_view(pkguse_line);
    pkg_constraint_str_view.remove_suffix(pkguse_line.size() - first_space_char);

    //pkg constraints cannot contain useflag constraints
    result.first = parse_pkg_constraint(pkg_constraint_str_view, false);

    if(not result.first.is_valid)
        return result;

    // create a view on the useflags "+foo -bar" and parse it
    string_view useflags_str_view(pkguse_line);
    useflags_str_view.remove_prefix(pkg_constraint_str_view.size());
    result.second = parse_useflags(useflags_str_view, true);

    return result;
}


PackageConstraint Parser::parse_pkg_constraint(string_view pkg_constraint_str, bool permit_useflags)
{
    /* Parses package constraint strings
     * e.g. "=app-misc/foo-1.2.3*:0=[-flag1(-),!flag2?,flag3]
     * */

    string_view str(pkg_constraint_str);

    PackageConstraint pkg_constraint;
    pkg_constraint.is_valid = true;

    skim_spaces_at_the_edges(str);

    // Check if there is useflag constraints
    if(str.ends_with(']'))
    {
        if(not permit_useflags)
        {
            pkg_constraint.is_valid = false;
            return pkg_constraint;
        }

        size_t start_index = str.find_first_of('[');
        if(start_index == string_view::npos or str.size() - start_index <= 2)
            throw runtime_error("Cannot parse package constraint " + string(pkg_constraint_str));

        pkg_constraint.flags = parse_pkg_useflag_constraints(str.substr(start_index + 1, str.size() - start_index - 2));

        str.remove_suffix(str.size() - start_index);
    }

    // Reset slot constraints to defaults and check if there is slot constraints

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

    // check for blockers
    pkg_constraint.blocker_type = PackageConstraint::BlockerType::NONE;
    if(str.starts_with("!!"))
    {
        str.remove_prefix(2);
        pkg_constraint.blocker_type = PackageConstraint::BlockerType::STRONG;
    }
    else if(str.starts_with('!'))
    {
        str.remove_prefix(1);
        pkg_constraint.blocker_type = PackageConstraint::BlockerType::WEAK;
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

        // TODO: deal with use flag and slots/subslot constraints

        pkg_constraint.ver.version.set_version(pkg_ver);
    }
    else
    {
        // there is no version constraint so it's simply the package name
        pkg_group_name = str;
    }

    pkg_constraint.pkg_id = pkgs->id_of(pkg_group_name);
    if(pkg_constraint.pkg_id == npos)
        pkg_constraint.is_valid = false;

    return pkg_constraint;
}

std::vector<UseflagConstraint> Parser::parse_pkg_useflag_constraints(string_view useflags_constraint_str)
{
    /* Parses the use flag constraint string inside a pkg dependency specification
     * e.g. it can receive "-flag1(-),!flag2?,flag3" as an input
     *      from ">=app-misc/foo-1.2.3:0=[-flag1(-),!flag2?,flag3]
     * */

    std::vector<UseflagConstraint> constraints;
    size_t next_comma;
    UseflagConstraint flagconstraint;
    flagconstraint.is_valid = true;

    string_view single_constraint;

    bool done = false;
    while(not done)
    {
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
            flagconstraint.type = UseflagConstraint::Type::CONDITIONAL;

            if(single_constraint.ends_with('?'))
            {
                single_constraint.remove_suffix(1);
                flagconstraint.forward_if_not_set = true;
                flagconstraint.forward_if_set = false;
                flagconstraint.forward_reverse_state = false;
            }
            else if(single_constraint.ends_with('='))
            {
                single_constraint.remove_suffix(1);
                flagconstraint.forward_if_not_set = true;
                flagconstraint.forward_if_set = true;
                flagconstraint.forward_reverse_state = true;
            }
            else throw runtime_error("Could not prase use flag constraint string: " + string(useflags_constraint_str));
        }
        else if(single_constraint.ends_with('?'))
        {
            flagconstraint.type = UseflagConstraint::Type::CONDITIONAL;
            single_constraint.remove_suffix(1);
            flagconstraint.forward_if_not_set = false;
            flagconstraint.forward_if_set = true;
            flagconstraint.forward_reverse_state = false;
        }
        else if(single_constraint.ends_with('='))
        {
            flagconstraint.type = UseflagConstraint::Type::CONDITIONAL;
            single_constraint.remove_suffix(1);
            flagconstraint.forward_if_not_set = true;
            flagconstraint.forward_if_set = true;
            flagconstraint.forward_reverse_state = false;
        }
        else
        {
            flagconstraint.type = UseflagConstraint::Type::DIRECT;
            flagconstraint.forward_if_not_set = false;
            flagconstraint.forward_if_set = false;
            flagconstraint.forward_reverse_state = false;

            flagconstraint.state = true;
            if(single_constraint.starts_with('-'))
            {
                flagconstraint.state = false;
                single_constraint.remove_prefix(1);
            }
            else if(single_constraint.starts_with('+'))
                single_constraint.remove_prefix(1);
        }

        flagconstraint.default_if_unexisting = false;
        if(single_constraint.ends_with("(+)"))
        {
            single_constraint.remove_suffix(3);
            flagconstraint.default_if_unexisting = true;
        }
        if(single_constraint.ends_with("(-)"))
            single_constraint.remove_suffix(3);

        flagconstraint.id = useflag_id(single_constraint, false);
        if(flagconstraint.id == npos)
        {
            flagconstraint.is_valid = false;
            cout << "Could not identify useflag: " + string(single_constraint) << endl;
        }

        constraints.emplace_back(flagconstraint);
    }

    return constraints;
}
