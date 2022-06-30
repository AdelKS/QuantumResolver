#include "string_utils.h"
#include "src/string_utils.h"

#include <filesystem>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cassert>

#include <chrono>
#include <regex>

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

/// \brief extracts the string line at 'line_num' from the string 'view'
/// \note if 'line_num' is bigger than the number of lines, it returns an empty string
std::string_view get_line(std::string_view view,  size_t line_num)
{
    size_t line = 0;
    size_t newline_pos = view.find('\n');    while(newline_pos != std::string_view::npos and line != line_num)
    {
        line++;
        view.remove_prefix(newline_pos+1);
        newline_pos = view.find('\n');
    };

    if(line != line_num)
        return std::string_view();
    else if(newline_pos != std::string_view::npos)
        return view.substr(0, newline_pos);
    else return view;
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

string_view get_next_word(string_view &words_line)
{
    /// \brief pops the first word from 'worlds_line' and returns it
    /// \brief making words_line smaller in the process
    /// \param words_line: string of "words" (random sequence of characters), separated by spaces
    /// \returns first word in 'words_line'
    /// \example "   hello there   " -> return "hello" and make words_line = "there"

    skim_spaces_at_the_edges(words_line);
    size_t next_space = words_line.find(' ');
    string_view ret;
    if(next_space == string_view::npos)
        words_line.swap(ret);
    else
    {
        ret = words_line.substr(0, next_space);
        words_line = words_line.substr(next_space + 1);
    }

    if(ret.empty())
        throw runtime_error("emtpy next word");

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

std::unordered_map<string, pair<size_t, size_t>> get_bash_vars(std::string_view str_view)
{
    std::unordered_map<string, pair<size_t, size_t>> bash_vars;

    size_t start_pos = str_view.find('$');
    while(start_pos != string_view::npos)
    {
        str_view.remove_prefix(start_pos + 1);

        if(str_view.size() <= 1)
            throw runtime_error("syntax error in make.defaults");

        size_t length = 0;
        bool found_bkt = str_view[0] == '{';
        if(found_bkt)
        {
            str_view.remove_prefix(1);
            length = str_view.find('}');
            if(length == str_view.npos)
                throw runtime_error("cannot find closing } to bash variable");
        }
        else length = str_view.find(' ');

        bash_vars.emplace(string(str_view.substr(0, length)),
                               make_pair(start_pos, length + 1 + 2*found_bkt));

        start_pos = str_view.find('$');
    }
    return bash_vars;
}

/// \brief returns the bounding rect of 'str'
/// \returns width and height of the string in number of chars
/// \note it removes the ansi escape before calculating the lengths
StringBoundingRect get_bouding_rect(const std::string& str)
{
    StringBoundingRect rect;
    std::string_view view(str);

    size_t newline_pos = view.find('\n');
    while(newline_pos != std::string_view::npos)
    {
        rect.height++;
        rect.width = std::max(rect.width,
                              plain_length(std::string(view.substr(0, newline_pos))));
        view.remove_prefix(newline_pos+1);
        newline_pos = view.find('\n');
    };

    rect.width = std::max(rect.width, plain_length(std::string(view)));
    return rect;
};

std::size_t get_min_width(std::string_view str_view)
{
    string plain_str = remove_ansi_escape(string(str_view));

    std::string_view view(plain_str);
    size_t min_width = 0;

    auto get_new_pos = [&]()
    {
        size_t new_pos = view.find('\n');
        new_pos = std::min(view.find(' '), new_pos);
        return new_pos;
    };

    size_t new_pos = get_new_pos();
    while(new_pos != std::string_view::npos)
    {
        min_width = std::max(min_width,
                             plain_length(std::string(view.substr(0, new_pos))));
        view.remove_prefix(new_pos+1);
        new_pos = get_new_pos();
    };

    if(min_width == 0)
        min_width = plain_str.size();

    return min_width;
}

string to_lower(string_view sv)
{
    string copy(sv);
    // TODO: update with ranges::for_each when it gest in gcc or clang
    for_each(copy.begin(), copy.end(), [](char& c){c = tolower(c, locale());});
    return copy;
}

string to_upper(string_view sv)
{
    string copy(sv);
    for_each(copy.begin(), copy.end(), [](char& c){c = toupper(c, locale());});
    return copy;
}

string wrap_indent(string_view view,
                        size_t max_length,
                        size_t wrap_indent)
{
    const size_t extra_lines = (view.size() / max_length);
    if(extra_lines == 0)
        return string(view);

    string res;
    res.reserve(view.size() + (view.size() / max_length) * (wrap_indent) );

    size_t current_length = 0;
    size_t newline_pos = view.find(' ');
    while(newline_pos != string_view::npos)
    {
        size_t char_length = plain_length(string(view.substr(0, newline_pos+1)));

        if(char_length > max_length)
            throw runtime_error("cannot wrap long line without white spaces");

        if(char_length + current_length > max_length)
        {
            res += "\n" + string(wrap_indent, ' ');
            current_length = wrap_indent;
        }
        current_length += char_length;

        res += view.substr(0, newline_pos+1);
        view.remove_prefix(newline_pos+1);
        newline_pos = view.find(' ');
    }

    res += view;

    return res;
}
