#include "utils.h"

#include <iostream>
#include <cstring>

using namespace std;

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

size_t pkg_namever_split_pos(const string &name_ver)
{
    bool found = false;
    size_t last_dash = 0, before_last_dash = 0;
    for(size_t i = 0 ; i < name_ver.size() ; i++)
        if(name_ver[i] == '-')
        {
            found = true;
            before_last_dash = last_dash;
            last_dash = i;
        }

    if(not found or last_dash + 1 == name_ver.size()) // last '-' cannot be at the end of the string
        throw runtime_error("Error splitting " + name_ver);
    else if(name_ver[last_dash+1] == 'r') // this a revision number
        return before_last_dash;
    else return last_dash;

}

vector<pair<size_t, string>> split_string(const string &str, const vector<string> &separators, const size_t first_variable_sep_index)
{
    /* splits string str size_to vector of pairs,
     * first_variable_sep_index is the index of the very first string that is split, who does not necessarily have a separator before it
     * Example:
     *  str="5.12.3_pre324_p32-r23", separators=[".", "_p", "_pre", "-r"], first_variable_sep_index=666
     *  the returned vector would be [(666, "5"), (0, "12"), (0, "3"), (2, "324"), (1, "32"), (3, "23")]
     *  where the first index of each couple is the index of the matched separator 0 -> ".", 1 -> "_p", 2 -> "_pre", 3 -> "-r
     * */

    const size_t N = str.size();
    size_t n;

    vector<pair<size_t, string>> split;

    size_t curr_sep_index = first_variable_sep_index;
    size_t new_sep_index = 0;
    size_t match_start = 0;
    bool match;

    for(size_t i = 0 ; i < N; i++)
    {
        match = false;

        for(size_t sep_index = 0 ; sep_index < separators.size() ; ++sep_index)
        {
            const string &sep = separators[sep_index];
            n = sep.size();

            if(n > N - i)
               continue;

            // Use strncmp to leverage SIMD instructions
            if(strncmp(str.c_str()+i, sep.c_str(), n) != 0)
                continue;

            if(not match)
            {
                match = not match;
                new_sep_index = sep_index;
            }
            else if(sep.size() > separators[new_sep_index].size())
                new_sep_index = sep_index;
        }

        if(match)
        {
            const string &new_sep = separators[new_sep_index];
            split.emplace_back(curr_sep_index, str.substr(match_start, i-match_start));
            match_start = i + new_sep.size();
            i += new_sep.size() - 1; // the for loop will increment that -1
            curr_sep_index = new_sep_index;
        }
    }
    if(match_start < N)
        split.emplace_back(curr_sep_index, str.substr(match_start, N-match_start));

    else if(match_start > N)
        throw runtime_error("string splitting failed, string=" + str);

    return split;
}


PackageConstraint parse_pkg_constraint(const string_view &pkg_constraint_str, const IndexedVector<string, Package> &pkgs)
{
    if(pkg_constraint_str.ends_with('*') and not pkg_constraint_str.starts_with('='))
        throw runtime_error("Met weird package version constraint: " + string(pkg_constraint_str));

    PackageConstraint pkg_constraint;
    string pkg_group_namever;
    string pkg_group_name;
    string pkg_ver;

    if(pkg_constraint_str.starts_with("<="))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::LESS;
        pkg_group_namever = pkg_constraint_str.substr(2);
    }
    else if(pkg_constraint_str.starts_with(">="))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::GREATER;
        pkg_group_namever = pkg_constraint_str.substr(2);
    }
    else if(pkg_constraint_str.starts_with('<'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SLESS;
        pkg_group_namever = pkg_constraint_str.substr(1);
    }
    else if(pkg_constraint_str.starts_with('>'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::SGREATER;
        pkg_group_namever = pkg_constraint_str.substr(1);
    }
    else if(pkg_constraint_str.starts_with('~'))
    {
        pkg_constraint.ver.type = VersionConstraint::Type::EQ_REV;
        pkg_group_namever = pkg_constraint_str.substr(1);
    }
    else if(pkg_constraint_str.starts_with('='))
    {
        if(pkg_constraint_str.ends_with('*'))
        {
            pkg_constraint.ver.type = VersionConstraint::Type::EQ_STAR;
            pkg_group_namever = pkg_constraint_str.substr(2, pkg_constraint_str.size()-4);
        }
        else
        {
            pkg_constraint.ver.type = VersionConstraint::Type::EQ;
            pkg_group_namever = pkg_constraint_str.substr(2);
        }
    }
    else
    {
        pkg_constraint.ver.type = VersionConstraint::Type::NONE;
        pkg_group_name = pkg_constraint_str;
    }

    if(pkg_constraint.ver.type != VersionConstraint::Type::NONE)
    {
        size_t split_pos = pkg_namever_split_pos(pkg_group_namever);
        pkg_group_name = pkg_group_namever.substr(0, split_pos);
        pkg_ver = pkg_group_namever.substr(split_pos+1);

        // TODO: deal with use flag and slots/subslot constraints

        pkg_constraint.ver.version.set_version_str(pkg_ver);
    }



    pkg_constraint.pkg_id = pkgs.index_of(pkg_group_name);
    if(pkg_constraint.pkg_id == pkgs.npos)
        cout << "Package not found while reading user per-package use flags: " + pkg_group_name << endl;

    return pkg_constraint;
}
