#include "misc_utils.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <cassert>

#include <chrono>

using namespace std;
using namespace chrono;
namespace fs = filesystem;

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

void skim_spaces_at_the_edges(string_view &str)
{
    // remove spaces at the beginning, if there are any
    // if str is made of only spaces, the resulting view is empty

    if(str.starts_with(' '))
        str.remove_prefix(min(str.find_first_not_of(' '), str.size()));

    // remove spaces at the end, if there are any
    if(not str.empty() and str.ends_with(' '))
        str.remove_suffix(str.size() - 1 - str.find_last_not_of(' '));
}

size_t pkg_namever_split_pos(const string_view &name_ver)
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
        throw runtime_error("Error splitting " + string(name_ver));
    else if(name_ver[last_dash+1] == 'r') // this a revision number
        return before_last_dash;
    else return last_dash;

}

vector<pair<size_t, string_view>> split_string(const string_view &str_view,
                                               const vector<string> &separators,
                                               const size_t first_variable_sep_index)
{
    /* splits string str size_to vector of pairs,
     * first_variable_sep_index is the index of the very first string that is split, who does not necessarily have a separator before it
     * Example:
     *  str="5.12.3_pre324_p32-r23", separators=[".", "_p", "_pre", "-r"], first_variable_sep_index=666
     *  the returned vector would be [(666, "5"), (0, "12"), (0, "3"), (2, "324"), (1, "32"), (3, "23")]
     *  where the first index of each couple is the index of the matched separator 0 -> ".", 1 -> "_p", 2 -> "_pre", 3 -> "-r
     * */

    const size_t N = str_view.size();
    size_t n;

    vector<pair<size_t, string_view>> split;

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

            if(not str_view.substr(i).starts_with(sep))
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
            split.emplace_back(curr_sep_index, str_view.substr(match_start, i-match_start));
            match_start = i + new_sep.size();
            i += new_sep.size() - 1; // the for loop will increment that -1
            curr_sep_index = new_sep_index;
        }
    }
    if(match_start < N)
        split.emplace_back(curr_sep_index, str_view.substr(match_start, N-match_start));

    else if(match_start > N)
        throw runtime_error("string splitting failed, string=" + string(str_view));

    return split;
}

string_view get_pth_enclosed_string_view(const string_view &str_view)
{
    // str_view needs to start with an open parenthesis '('
    // returns the string between the character after the opening parenthesis
    // till the corresponding closing parenthesis ')' (not included)

    if(not str_view.starts_with('('))
        throw runtime_error("String does not start with a parenthesis: " + string(str_view));

    size_t pth_num = 1;
    size_t index = 0; // index is incremented first

    while(pth_num != 0 and index < str_view.size())
    {
        index++;
        if(str_view[index] == '(')
            pth_num++;
        else if(str_view[index] == ')')
            pth_num--;
    }

    if(index == str_view.size())
        throw runtime_error("No closing parenthesis in this string: " + string(str_view));

    return str_view.substr(1, index-1);
}

vector<fs::path> get_regular_files(const fs::path &path)
{
    vector<fs::path> regular_files;
    if(fs::is_regular_file(path))
        regular_files.push_back(path);
    else if(fs::is_directory(path))
        for(const auto &file: fs::directory_iterator(path))
            if(fs::is_regular_file(file))
                regular_files.push_back(file);

    return regular_files;
}

vector<string> read_file_lines(const filesystem::path& file_path)
{
    /* Reads the lines of the file referenced by file_path
     * and returns them.
     * Note: Spaces at the beginning and end of each line are removed
     * */
    fstream file(file_path, ios::in);
    if(not file.is_open())
        throw runtime_error("Couldn't open parent file" + file_path.string());

    char line[LINE_MAX_SIZE];
    vector<string> file_lines;

    while(file.getline(line, LINE_MAX_SIZE, '\n'))
    {
        string_view view(line);
        skim_spaces_at_the_edges(view);
        if(view.starts_with("#") or view.empty())
            continue;

        file_lines.emplace_back(view);
    }

    return file_lines;
}

template <bool quoted, class... StartWithContiner> requires (sizeof...(StartWithContiner) == 0 or
                                                              (sizeof...(StartWithContiner) == 1 and
                                                                 (std::is_same_v<StartWithContiner, vector<string>> and ...)))
auto read_vars(const filesystem::path& file_path,
                                       const StartWithContiner& ... start_with)
{
    auto get_return_type = []()
    {
        if constexpr (sizeof...(StartWithContiner) == 1)
            return unordered_map<string, string>();
        else return vector<pair<string, string>>();
    };

    auto vars = get_return_type();

    auto emplace_result = [&vars](string v1, string v2)
    {
        if(v2.starts_with("rapi"))
            cout << "here!";
        if constexpr (sizeof...(StartWithContiner) == 1)
            vars.emplace(move(v1), move(v2));
        else vars.emplace_back(move(v1), move(v2));
    };

    const auto &file_lines = read_file_lines(file_path);
    for(const string &line: file_lines)
    {
        size_t eq_sign_pos = line.find('=');

        if(eq_sign_pos == string_view::npos)
            continue;

        if constexpr (sizeof...(StartWithContiner) == 1)
        {
            auto get_start_with_list = []<class... Elems>(const vector<string> &vec) {return vec;};
            const vector<string>& start_with_list = get_start_with_list(start_with...);
            bool found = false;
            for(size_t i = 0 ; i < start_with_list.size() and not found ; i++)
                found = line.starts_with(start_with_list[i]);

            if(not found)
                continue;
        }

        if constexpr (quoted)
        {
            if(not (eq_sign_pos < line.size() - 2 and // to make sure that the line doesn't end with ="
            line[eq_sign_pos+1] == '"' and
            line.back() == '"'))
                throw runtime_error("File " + file_path.string() + " has problems in its variable definition");
        }

        emplace_result
        (
            line.substr(0, eq_sign_pos),
            line.substr(eq_sign_pos + 1 + quoted, line.size() - 1 - eq_sign_pos - 2*quoted)
        );
    }

    return vars;
}

unordered_map<string, string> read_quoted_vars(const filesystem::path& file_path,
                                              const vector<string>& start_with_list)
{
    return read_vars<true>(file_path, start_with_list);
}

unordered_map<string, string> read_unquoted_vars(const filesystem::path& file_path,
                                              const vector<string>& start_with_list)
{
    return read_vars<false>(file_path, start_with_list);
}

vector<pair<string, string>> read_quoted_vars(const filesystem::path& file_path)
{
    return read_vars<true>(file_path);
}

vector<pair<string, string>> read_unquoted_vars(const filesystem::path& file_path)
{
    return read_vars<false>(file_path);
}

string_view get_next_word(string_view &words_line)
{
    // pops the first words from worlds_line and returns it
    // making words_line smaller in the process
    // e.g. "   hello there   " -> return "hello" and make words_line = "there"

    skim_spaces_at_the_edges(words_line);
    if(words_line.empty())
        return string_view();

    size_t next_space = words_line.find(' ');
    string_view ret;
    if(next_space == string_view::npos)
        words_line.swap(ret);
    else
    {
        ret = words_line.substr(0, next_space);
        words_line = words_line.substr(next_space + 1);
    }

    return ret;
}

unordered_set<size_t> get_activated_useflags(unordered_map<size_t, bool> flag_states)
{
    unordered_set<size_t> activated_flags;
    for(const auto &[flag_id, flag_state]: flag_states)
    {
        if(flag_state)
            activated_flags.insert(flag_id);
    }
    return activated_flags;
}

const vector<fs::path> &get_profiles_tree()
{
    fs::path profile_symlink("/etc/portage/make.profile");
    if(not fs::is_symlink(profile_symlink))
        throw runtime_error("/etc/portage/make.profile doesn't exist or isn't a symlink");

    fs::path profile = fs::canonical(profile_symlink);
    cout << "Absolute path of the profile " << profile.string() << endl;

    cout << "Populating profile tree" << endl;
    auto start = high_resolution_clock::now();

    static vector<fs::path> profile_tree = {"/etc/portage", profile};
    static bool tree_already_populated = false;

    if(not tree_already_populated)
    {
        tree_already_populated = true;
        deque<fs::path> explore_queue = {profile};

        while(not explore_queue.empty())
        {
            const fs::path curr_profile = explore_queue.back(); explore_queue.pop_back();

            const fs::path &parent_file_path(curr_profile.string() + "/parent");
            if(fs::is_regular_file(parent_file_path))
            {
                deque<fs::path> new_profiles;
                // Invert the list of explored profiles in the parent file
                // so the first line of "parent" is explored first
                for(const string &line: read_file_lines(parent_file_path))
                {
                    const auto &new_profile = fs::canonical(fs::path(curr_profile.string() + "/" + line + "/"));
                    new_profiles.push_front(new_profile);
                }
                for(const auto &profile: new_profiles)
                {
                    profile_tree.push_back(profile);
                    explore_queue.push_back(profile);
                }
            }
        }
    }

    auto end = high_resolution_clock::now();
    cout << "duration : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;

    return profile_tree;
}

string to_lower(string_view sv)
{
    string copy(sv);
    // TODO: update with ranges::for_each when it gest in gcc or clang
    for_each(copy.begin(), copy.end(), [](char& c){c = tolower(c, locale());});
    return copy;
}

string to_lower(string&& sv)
{
    for_each(sv.begin(), sv.end(), [](char& c){c = tolower(c, locale());});
    return sv;
}

string to_upper(string_view sv)
{
    string copy(sv);
    for_each(copy.begin(), copy.end(), [](char& c){c = toupper(c, locale());});
    return copy;
}

string to_upper(string&& sv)
{
    for_each(sv.begin(), sv.end(), [](char& c){c = toupper(c, locale());});
    return sv;
}

unordered_set<size_t> operator + (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    unordered_set<size_t> ret(a);
    ret.insert(b.begin(), b.end());
    return ret;
}

unordered_set<size_t> operator + (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    a.insert(b.begin(), b.end());
    return a;
}

unordered_set<size_t> operator & (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    const unordered_set<size_t>& smaller = a.size() <= b.size() ? a : b;
    const unordered_set<size_t>& bigger = a.size() > b.size() ? a : b;

    unordered_set<size_t> ret;
    for(size_t val: smaller)
        if(bigger.contains(val))
            ret.insert(val);

    return ret;
}

unordered_set<size_t> operator & (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    for(auto it = a.begin(); it != a.end();)
    {
       if(not b.contains(*it))
          it = a.erase(it);
       else ++it;
    }

    return a;
}

unordered_set<size_t> operator - (const unordered_set<size_t>& a,
                                            const unordered_set<size_t>& b)
{
    if(a.size() <= b.size())
    {
        unordered_set<size_t> ret;
        for(size_t val: a)
            if(not b.contains(val))
                ret.insert(val);

        return ret;
    }
    else
    {
        unordered_set<size_t> ret(a);
        for(size_t val: b)
            ret.erase(val);

        return ret;
    }
}

unordered_set<size_t> operator - (unordered_set<size_t>&& a,
                                            const unordered_set<size_t>& b)
{
    for(auto it = a.begin(); it != a.end();)
    {
       if(b.contains(*it))
          it = a.erase(it);
       else ++it;
    }

    return a;
}
