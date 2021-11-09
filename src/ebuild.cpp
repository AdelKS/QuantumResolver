#include "ebuild.h"

#include "parseutils.h"

using namespace std;

Ebuild::Ebuild(const string &ver,
               const filesystem::path &path,
               shared_ptr<Parser> parser):
    eversion(ver), ebuild_path(path), parser(parser), masked(false)
{
    ebuild_lines = read_file_lines(ebuild_path, {"IUSE", "DEPEND"});
}

void Ebuild::parse_dep_string()
{
    for(string_view ebuild_line_view: ebuild_lines)
    {
        if(ebuild_line_view.starts_with("DEPEND"))
        {
            ebuild_line_view.remove_prefix(7);
            add_deps(parse_dep_string(ebuild_line_view), Dependencies::Type::BUILD);
        }
        else if(ebuild_line_view.starts_with("BDEPEND"))
        {
            ebuild_line_view.remove_prefix(8);
            add_deps(parse_dep_string(ebuild_line_view), Dependencies::Type::BUILD);
        }
        else if(ebuild_line_view.starts_with("RDEPEND"))
        {
            ebuild_line_view.remove_prefix(8);
            add_deps(parse_dep_string(ebuild_line_view), Dependencies::Type::RUNTIME);
        }
        else if(ebuild_line_view.starts_with("PDEPEND"))
        {
            ebuild_line_view.remove_prefix(8);
            add_deps(parse_dep_string(ebuild_line_view), Dependencies::Type::RUNTIME);
        }
    }
}

void Ebuild::parse_iuse()
{
    for(string_view ebuild_line_view: ebuild_lines)
    {
        if(ebuild_line_view.starts_with("IUSE"))
        {
            // shrink by 5 to remove the "IUSE=" in the beginning
            ebuild_line_view.remove_prefix(5);
            auto flag_states = parser->parse_useflags(ebuild_line_view, false, true);
            add_useflags(flag_states);
            break;
        }
    }
}

void Ebuild::add_useflag(size_t flag_id, bool default_state)
{
    ebuild_useflags[flag_id] = default_state;
}

void Ebuild::add_useflags(std::unordered_map<std::size_t, bool> useflags_and_default_states)
{
    for(const auto &flag_state_iter: useflags_and_default_states)
    {
        add_useflag(flag_state_iter.first, flag_state_iter.second);
    }
}

void Ebuild::assign_useflag_state(size_t flag_id, bool state)
{
    auto flag_iter = ebuild_useflags.find(flag_id);
    if(flag_iter != ebuild_useflags.end())
        flag_iter->second = state;

}

void Ebuild::assign_useflag_states(std::unordered_map<std::size_t, bool> useflag_states)
{
    for(const auto &flag_state_iter: useflag_states)
    {
        assign_useflag_state(flag_state_iter.first, flag_state_iter.second);
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
    return eversion < other.eversion;
}

const EbuildVersion &Ebuild::get_version()
{
    return eversion;
}

void Ebuild::add_deps(const Dependencies &deps, Dependencies::Type dep_type)
{
    Dependencies &m_deps = dep_type == Dependencies::Type::BUILD ? bdeps : rdeps;

    if(not deps.valid)
    {
        masked = true;
        return;
    }

    for(const auto &dep: deps.or_deps)
        m_deps.or_deps.push_back(dep);

    for(const auto &dep: deps.plain_deps)
        m_deps.plain_deps.push_back(dep);

    for(const auto &dep: deps.use_cond_deps)
        m_deps.use_cond_deps.push_back(dep);

    for(const auto &dep: deps.xor_deps)
        m_deps.xor_deps.push_back(dep);
}

Dependencies Ebuild::parse_dep_string(string_view dep_string)
{
    /* Receives a dependency string as formatted in files in /var/db/repos/gentoo/metadata/md5-cache/
     * e.g. || ( dev-lang/python:3.10[ncurses,sqlite,ssl] dev-lang/python:3.9[ncurses,sqlite,ssl] dev-lang/python:3.8[ncurses,sqlite,ssl] ) app-arch/unzip
     */

    Dependencies deps;
    deps.valid = true;

    string original_string(dep_string);

    skim_spaces_at_the_edges(dep_string);

    while(not dep_string.empty())
    {
        if(dep_string.starts_with("|| ( "))
        {
            // remove till the beginning of the parenthesis
            dep_string.remove_prefix(3);

            // retrieve the enclosed content and add it to "or" deps
            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.or_deps.emplace_back(parse_dep_string(enclosed_string));
            if(not deps.or_deps.back().valid)
            {
                deps.valid = false;
                return deps;
            }

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else if(dep_string.starts_with("^^ ( "))
        {
            // remove till the beginning of the parenthesis
            dep_string.remove_prefix(3);

            // retrieve the enclosed content and add it to "or" deps
            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.xor_deps.emplace_back(parse_dep_string(enclosed_string));
            if(not deps.xor_deps.back().valid)
            {
                deps.valid = false;
                return deps;
            }

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else if(dep_string.starts_with("?? ( "))
        {
            // remove till the beginning of the parenthesis
            dep_string.remove_prefix(3);

            // retrieve the enclosed content and add it to "or" deps
            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.at_most_one_deps.emplace_back(parse_dep_string(enclosed_string));
            if(not deps.at_most_one_deps.back().valid)
            {
                deps.valid = false;
                return deps;
            }

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else if(dep_string.starts_with('('))
        {
            // it's an all of group

            const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
            deps.all_of_deps.emplace_back(parse_dep_string(enclosed_string));
            if(not deps.all_of_deps.back().valid)
            {
                deps.valid = false;
                return deps;
            }

            // move prefix to skip the enclosed content +2 to remove the parentheses
            dep_string.remove_prefix(enclosed_string.size() + 2);
        }
        else
        {
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
                Toggle flag_cond;

                flag_cond.state = true;
                if(constraint.starts_with('!'))
                {
                    constraint.remove_prefix(1);
                    flag_cond.state = false;
                }

                flag_cond.id = parser->useflag_id(constraint, false);
                if(flag_cond.id == npos)
                {
                    deps.valid = false;
                    return deps;
                }

                // retrieve the enclosed content and add it to "or" deps
                const string_view &enclosed_string = get_pth_enclosed_string_view(dep_string);
                deps.use_cond_deps.emplace_back(make_pair(flag_cond, parse_dep_string(enclosed_string)));
                if(not deps.use_cond_deps.back().second.valid or deps.use_cond_deps.back().first.id == npos)
                {
                    deps.valid = false;
                    return deps;
                }

                // move prefix to skip the enclosed content +2 to remove the parentheses
                dep_string.remove_prefix(enclosed_string.size() + 2);
            }
            else
            {
                // it is a "plain" (this may be a nested call) pkg constraint
                const PackageConstraint& pkg_constraint = parser->parse_pkg_constraint(constraint);
                if(pkg_constraint.is_valid)
                    deps.plain_deps.emplace_back(pkg_constraint);
                else if(pkg_constraint.blocker_type == PackageConstraint::BlockerType::NONE)
                {
                    cout << "Could not find package: " << string(constraint) << endl;
                    deps.valid = false;
                    return deps;
                }
            }
        }

        skim_spaces_at_the_edges(dep_string);
    }

    return deps;
}


