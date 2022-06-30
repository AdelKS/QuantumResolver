#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cctype>

#include "concepts.h"

#define LINE_MAX_SIZE 50000


std::string exec(const char* cmd);

std::size_t pkg_namever_split_pos(const std::string_view &name_ver);

void skim_spaces_at_the_edges(std::string_view &str);

std::string_view get_pth_enclosed_string_view(const std::string_view &str_view);

/// \brief reads string and makes a upper case copy
std::string to_lower(std::string_view sv);

/// \brief reads string and makes a upper case copy
std::string to_upper(std::string_view sv);

std::string_view get_next_word(std::string_view &words_line);

std::vector<std::pair<std::size_t, std::string_view>> split_string(const std::string_view &str_view,
                                                              const std::vector<std::string> &separators,
                                                              const std::size_t first_variable_sep_index = 0);

std::unordered_set<std::size_t> get_activated_useflags(std::unordered_map<std::size_t, bool> flag_states);



inline std::string remove_ansi_escape(const std::string& str)
{
    // taken from https://stackoverflow.com/questions/14693701/how-can-i-remove-the-ansi-escape-sequences-from-a-string-in-python
    const static std::regex reg(R"MERDE(\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~]))MERDE");
    return std::regex_replace(str, reg, "");
}

inline size_t plain_length(const std::string& str)
{
    return remove_ansi_escape(str).size();
}

struct StringBoundingRect
{
    size_t width=1, height=1;
};

/// \brief returns the bounding rect of 'str'
/// \returns width and height of the string in number of chars
/// \note it removes the ansi escape before calculating the lengths
StringBoundingRect get_bouding_rect(const std::string& str);

std::size_t get_min_width(std::string_view view);

/// \brief finds the bash variables in 'str_view', i.e. of the form $foo and/or ${bar}
/// \param str_view: the string on which to look for bash variables
/// \returns unordered_map that maps plain variable name (without $ nor {}) to (start_pos, length) in the string_view
/// \example "$foo ${bar}" -> { {"foo", {0, 4}}, {"bar", {5, 6}} }
std::unordered_map<std::string, std::pair<std::size_t, std::size_t>> get_bash_vars(std::string_view str_view);

template <StringRange StringList>
std::vector<std::vector<std::string_view>> split_string_list(const StringList& str_list, size_t max_length = 100)
{
    std::vector<std::vector<std::string_view>> res;
    size_t current_length = 0;
    std::vector<std::string_view> current_line;
    for(const std::string& val: str_list)
    {
        size_t element_size = remove_ansi_escape(val).size();

        if(element_size >= max_length)
            throw std::runtime_error("Met a single element that is longer than max_length");

        if(current_length + element_size > max_length)
        {
            res.push_back(std::move(current_line));
            current_length = 0;
        }

        current_line.push_back(val);
        current_length += element_size;
    }
    res.push_back(std::move(current_line));
    return res;
}

/// \brief extracts the string line at 'line_num' from the string 'view'
/// \note if 'line_num' is bigger than the number of lines, it returns an empty string
std::string_view get_line(std::string_view view,  size_t line_num);

/// \brief wraps the string at white spaces before max_length is reached
/// \note  throws if it's not possible, skims ansi escape codes to treat
///        as if it's a plain string
/// \param view: string to wrap
/// \param max_length: length to not exceed before wrapping
/// \param wrap_indent: additionnal indent for new lines
std::string wrap_indent(std::string_view view,
                        size_t max_length = 70,
                        size_t wrap_indent = 0);

template <StringRange StringList>
std::string concatenate(const StringList& str_list, const std::string_view& sep)
{
    std::string res;
    size_t final_size = 0;
    for(const auto& str: str_list)
        final_size += str.size() + sep.size();

    res.reserve(final_size - sep.size());
    size_t i = 0;
    for(const auto& str: str_list)
    {
        res += str;
        if(++i != str_list.size())
            res += sep;
    }
    return res;
}

#endif // STRING_UTILS_H
