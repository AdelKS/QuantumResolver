#include "parseutils.h"

#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;
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

void skim_spaces_at_the_edges(std::string_view &str)
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

vector<pair<size_t, string>> split_string(const string &str, const vector<string> &separators, const size_t first_variable_sep_index)
{
    /* splits string str size_to vector of pairs,
     * first_variable_sep_index is the index of the very first string that is split, who does not necessarily have a separator before it
     * Example:
     *  str="5.12.3_pre324_p32-r23", separators=[".", "_p", "_pre", "-r"], first_variable_sep_index=666
     *  the returned vector would be [(666, "5"), (0, "12"), (0, "3"), (2, "324"), (1, "32"), (3, "23")]
     *  where the first index of each couple is the index of the matched separator 0 -> ".", 1 -> "_p", 2 -> "_pre", 3 -> "-r
     * */

    // TODO: use string views and starts_with()

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

deque<fs::path> get_regular_files(const fs::path &path)
{
    deque<fs::path> regular_files;
    if(fs::is_regular_file(path))
        regular_files.push_back(path);
    else if(fs::is_directory(path))
        for(const auto &file: fs::directory_iterator(path))
            if(fs::is_regular_file(file))
                regular_files.push_back(file);

    return regular_files;
}

deque<string> read_file_lines(const fs::path file_path, bool omit_comments_and_empty, size_t max_line_size)
{
    /* Reads the lines of the file referenced by file_path
     * and returns them.
     * Note: Spaces at the beginning and end of each line are removed
     * */
    fstream file(file_path, ios::in);
    if(not file.is_open())
        throw runtime_error("Couldn't open parent file" + file_path.string());

    char line[max_line_size];
    deque<string> file_lines;

    while(file.getline(line, max_line_size, '\n'))
    {
        string_view view(line);
        skim_spaces_at_the_edges(view);
        if(not omit_comments_and_empty or (not view.starts_with("#") and not view.empty()))
            file_lines.emplace_back(string(view));
    }

    return file_lines;
}
