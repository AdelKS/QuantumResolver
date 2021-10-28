#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <chrono>
using namespace std::chrono;

#include "database.h"
#include "utils.h"

#define LINE_MAX_SIZE 10000

namespace fs = filesystem;
using namespace std;

Database::Database()
{
}

size_t Database::get_useflag_id(const string &useflag_str, bool create_ids)
{
    /* Returns the id of the useflag given by useflag_str
     * if create_ids = true, it creates an id if it doesn't exist already
     * if create_ids = false and the string hasn't been encountered before, returns npos
     */

    // TODO: figure out how to use string_view for performance
    //       c.f. https://en.cppreference.com/w/cpp/container/unordered_map/find

    size_t useflag_id = npos;
    auto use_flag_it = useflag_str_to_id.find(useflag_str);
    if(use_flag_it == useflag_str_to_id.end())
    {
        if(create_ids)
        {
            useflag_id = useflag_str_to_id.size();
            useflag_str_to_id.insert(make_pair(useflag_str, useflag_id));
        }
    }
    else useflag_id = use_flag_it->second;

    return useflag_id;
}

std::unordered_map<std::size_t, bool> Database::parse_useflags(const string_view &useflags_str, bool default_state, bool create_ids)
{
    auto start_it = useflags_str.begin(), end_it = useflags_str.begin();

    std::unordered_map<std::size_t, bool> parsed_useflags;

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
                size_t useflag_id = get_useflag_id(string(string_view(start_it, end_it)), create_ids);
                if(useflag_id == npos)
                    cout << "This useflag doesn't exist: " + string(string_view(start_it, end_it)) << endl;

                // add flag id and its state to the list
                parsed_useflags.insert(make_pair(useflag_id, state));

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

std::vector<UseflagConstraint> Database::parse_useflag_constraints(string_view useflags_constraint_str)
{

    std::vector<UseflagConstraint> constraints;
    size_t next_comma;
    UseflagConstraint flagconstraint;
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

        flagconstraint.id = get_useflag_id(string(single_constraint));
        if(flagconstraint.id == npos)
            throw runtime_error("Could not identify useflag: " + string(single_constraint));

        constraints.push_back(flagconstraint);
    }

    return constraints;
}

void Database::account_for_global_useflags()
{
    string global_useflags_str = exec("portageq envvar USE");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    global_useflags_str += " " + exec("portageq envvar IUSE_IMPLICIT");
    if(global_useflags_str.ends_with('\n'))
        global_useflags_str.pop_back();

    // All use flags are contained in ebuilds in md5-cache, do not create new ones from profile
    auto global_useflags = parse_useflags(global_useflags_str, true, false);
    VersionConstraint constraint;
    constraint.type = VersionConstraint::Type::NONE;

    for(size_t i = 0 ; i < pkgs.size(); i++)
        pkgs[i].update_useflags_with_constraints(constraint, global_useflags);
}

void Database::account_for_user_useflags()
{
    vector<fs::path> userflag_file_paths;

    fs::path user_useflags_path("/etc/portage/package.use");
    if(fs::is_directory(user_useflags_path))
    {
        for(fs::directory_entry const& entry: fs::recursive_directory_iterator(user_useflags_path))
        {
            if(entry.is_regular_file())
                userflag_file_paths.emplace_back(entry.path());
        }
    }
    else if(fs::is_regular_file(user_useflags_path))
    {
        userflag_file_paths.emplace_back(user_useflags_path);
    }
    else throw runtime_error("/etc/portage/package.use is weird");

    char line[LINE_MAX_SIZE];
    size_t first_space_char;

    for(const auto &path: userflag_file_paths)
    {
        fstream file(path, ios::in);
        if(not file.is_open())
            throw runtime_error("Couldn't open file" + path.string());

        while(file.getline(line, LINE_MAX_SIZE, '\n'))
        {
            string_view line_str_view(line);

            // remove eventual spurious spaces
            skim_spaces_at_the_edges(line_str_view);

            if(line_str_view.empty() or line_str_view.starts_with('#'))
                continue;

            // find the first space that separates the package specification from the use flags
            // e.g.      >=app-misc/foo-1.2.3 +foo -bar
            //           first space here:   ^

            first_space_char = line_str_view.find(' ');
            if(first_space_char == string_view::npos)
                throw runtime_error("In file: " + path.string() + "\n Cannot parse line: " + string(line));

            // create a view on the package constraint str ">=app-misc/foo-1.2.3" then parse it
            string_view pkg_constraint_str_view(line_str_view);
            pkg_constraint_str_view.remove_suffix(line_str_view.size() - first_space_char);
            auto pkg_constraint = parse_pkg_constraint(pkg_constraint_str_view);

            if(pkg_constraint.pkg_id == pkgs.npos)
                continue;

            // create a view on the useflags "+foo -bar" and parse it
            string_view useflags_str_view(line_str_view);
            useflags_str_view.remove_prefix(pkg_constraint_str_view.size());
            auto useflags = parse_useflags(useflags_str_view, true);

            // give that to the relevant package so it updates its ebuilds
            pkgs[pkg_constraint.pkg_id].update_useflags_with_constraints(pkg_constraint.ver, useflags);
        }
    }
}

Dependencies Database::parse_dependencies(string_view dep_string)
{
    Dependencies deps;

    string_view original_string = dep_string;

    while(not dep_string.empty())
    {
        skim_spaces_at_the_edges(dep_string);

        if(dep_string.starts_with("|| ( "))
        {
            // remove till the beginning of the parenthesis
            dep_string.remove_prefix(3);

            // retrieve the enclosed content and add it to "or" deps
            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.or_deps.push_back(parse_dependencies(enclosed_string));

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }

        // it's okay if it returns npos
        size_t count = dep_string.find_first_of(' ');
        string_view constraint = dep_string.substr(0, count);
        dep_string.remove_prefix(constraint.size());

        if(constraint.ends_with('?'))
        {
            // This is flag condition

            if(count == string_view::npos)
                throw runtime_error("Use condition at the end of dep string: " + string(dep_string));

            dep_string.remove_prefix(1);
            if(not dep_string.starts_with('('))
                throw runtime_error("No opening parenthesis after use flag condition in dep string: " + string(dep_string));

            constraint.remove_suffix(1);
            UseflagCondition flag_cond;

            flag_cond.state = true;
            if(constraint.starts_with('!'))
            {
                constraint.remove_prefix(1);
                flag_cond.state = false;
            }

            flag_cond.flag_id = get_useflag_id(string(constraint));
            if(flag_cond.flag_id == npos)
                throw runtime_error("Flag not recognized: " + string(constraint));

            // retrieve the enclosed content and add it to "or" deps
            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.use_cond_deps.push_back(make_pair(flag_cond, parse_dependencies(enclosed_string)));

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else
        {
            // it is a "plain" (this may be a nested call) pkg constraint
            deps.plain_deps.push_back(parse_pkg_constraint(constraint));
        }

    }

    return deps;
}

void Database::populate_from_cache_dir(string path)
{
    fs::path cache_path(path);
    if(not fs::is_directory(cache_path))    
        throw runtime_error("Path is not a directory");

    string pkg_group, pkg_namever, pkg_name, pkg_ver;
    size_t split_pos;

    char ebuild_line[LINE_MAX_SIZE];

    // store dep strings then parse them later, we need to populate the useflags first
    // we read each file in the cache dir exactly _once_
    vector<pair<Ebuild*, string>> deps, rdeps, bdeps, pdeps;

    for(fs::directory_entry const& entry: fs::recursive_directory_iterator(cache_path))
    {
        if(not (entry.is_regular_file() and entry.path().parent_path().parent_path() == cache_path))
            continue;

        pkg_namever = entry.path().filename(); // we use filename here because the cache files do not contain any extension
        pkg_group = entry.path().parent_path().stem();

        split_pos = pkg_namever_split_pos(pkg_namever);

        pkg_name = pkg_namever.substr(0, split_pos);
        pkg_ver = pkg_namever.substr(split_pos+1);

        Package &pkg = pkgs[pkg_group + "/" + pkg_name]; // created if doesn't exist
        auto &ebuild = pkg.add_version(pkg_ver);

        fstream file(entry.path(), ios::in);
        if(not file.is_open())
            throw runtime_error("Couldn't open file" + entry.path().string());

        while(file.getline(ebuild_line, LINE_MAX_SIZE, '\n'))
        {
            string_view ebuild_line_view(ebuild_line);
            if(ebuild_line_view.starts_with("IUSE"))
            {
                // shrink by 5 to remove the "IUSE=" in the beginning
                ebuild_line_view.remove_prefix(5);
                auto flag_states = parse_useflags(ebuild_line_view, false, true);
                ebuild.add_useflags(flag_states);
            }
            else if(ebuild_line_view.starts_with("DEPEND"))
            {
                ebuild_line_view.remove_prefix(7);
                deps.push_back(make_pair(&ebuild, string(ebuild_line_view)));
            }
            else if(ebuild_line_view.starts_with("BDEPEND"))
            {
                ebuild_line_view.remove_prefix(8);
                bdeps.push_back(make_pair(&ebuild, string(ebuild_line_view)));
            }
            else if(ebuild_line_view.starts_with("RDEPEND"))
            {
                ebuild_line_view.remove_prefix(8);
                rdeps.push_back(make_pair(&ebuild, string(ebuild_line_view)));
            }
            else if(ebuild_line_view.starts_with("PDEPEND"))
            {
                ebuild_line_view.remove_prefix(8);
                pdeps.push_back(make_pair(&ebuild, string(ebuild_line_view)));
            }
        }
    }

    // TODO: update this by reading KEYWORDS
    get_useflag_id("amd64", true); get_useflag_id("x86", true);


    // parse deps
    auto start = high_resolution_clock::now();

    for(auto &ebuild_depstring: deps)
        ebuild_depstring.first->add_deps(parse_dependencies(ebuild_depstring.second), Dependencies::Type::BUILD);
    for(auto &ebuild_depstring: bdeps)
        ebuild_depstring.first->add_deps(parse_dependencies(ebuild_depstring.second), Dependencies::Type::BUILD);
    for(auto &ebuild_depstring: rdeps)
        ebuild_depstring.first->add_deps(parse_dependencies(ebuild_depstring.second), Dependencies::Type::RUNTIME);
    for(auto &ebuild_depstring: pdeps)
        ebuild_depstring.first->add_deps(parse_dependencies(ebuild_depstring.second), Dependencies::Type::RUNTIME);

    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "It took " << duration.count() << "ms to parse all deps in ::gentoo" << endl;

    account_for_global_useflags();
    account_for_user_useflags();
}

PackageConstraint Database::parse_pkg_constraint(string_view pkg_constraint_str)
{
    // TODO: differentiate between pkg constraints in /etc/portage/package.use and DEPEND vars
    //       the latter can have [flag] stuff.

    string_view str(pkg_constraint_str);

    PackageConstraint pkg_constraint;

    skim_spaces_at_the_edges(str);

    // Check if there is useflag constraints
    if(str.ends_with(']'))
    {
        size_t start_index = str.find_first_of('[');
        if(start_index == string_view::npos or str.size() - start_index <= 2)
            throw runtime_error("Cannot parse package constraint " + string(pkg_constraint_str));

        pkg_constraint.flags = parse_useflag_constraints(str.substr(start_index + 1, str.size() - start_index - 2));

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
        pkg_group_namever = str.substr(2);
    }
    else if(str.starts_with(">="))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::GREATER;
        pkg_group_namever = str.substr(2);
    }
    else if(str.starts_with('<'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SLESS;
        pkg_group_namever = str.substr(1);
    }
    else if(str.starts_with('>'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SGREATER;
        pkg_group_namever = str.substr(1);
    }
    else if(str.starts_with('~'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::EQ_REV;
        pkg_group_namever = str.substr(1);
    }
    else if(str.starts_with('='))
    {
        if(str.ends_with('*'))
        {
            pkg_constraint.ver.type = VersionConstraint::Type::EQ_STAR;
            pkg_group_namever = str.substr(1, str.size()-2);
        }
        else
        {
            pkg_constraint.ver.type = VersionConstraint::Type::EQ;
            pkg_group_namever = str.substr(1);
        }
    }
    else
    {
        pkg_constraint.ver.type = VersionConstraint::Type::NONE;
        pkg_group_name = str;
    }

    if(pkg_constraint.ver.type != VersionConstraint::Type::NONE)
    {
        size_t split_pos = pkg_namever_split_pos(pkg_group_namever);
        pkg_group_name = pkg_group_namever.substr(0, split_pos);
        pkg_ver = pkg_group_namever.substr(split_pos+1);

        // TODO: deal with use flag and slots/subslot constraints

        pkg_constraint.ver.version.set_version(pkg_ver);
    }

    pkg_constraint.pkg_id = pkgs.index_of(pkg_group_name);
    if(pkg_constraint.pkg_id == pkgs.npos)
        cout << "Package not found while reading user per-package use flags: " + pkg_group_name << endl;

    return pkg_constraint;
}
