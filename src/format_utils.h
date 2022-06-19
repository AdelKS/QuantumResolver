#pragma once

#include "concepts.h"
#include "src/misc_utils.h"
#include "src/parser.h"
#include "useflags.h"
#include <string>
#include <string_view>
#include <unordered_map>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/color.h>

constexpr fmt::rgb gentoo_blue(18,72,139);
constexpr fmt::rgb gentoo_red(192,28,40);
constexpr fmt::rgb gentoo_green(38,162,105);
constexpr fmt::rgb gentoo_orange = fmt::color::dark_orange;
constexpr fmt::rgb gentoo_black = fmt::color::black;

/// \brief generates a colored and ordered formatting from the 'enabled' and 'disabled' useflag IDs
///        and uses the 'enforced' and 'changed' sets to decide if the flags need to be
///        between parentheses or, respectively, colored in green with an added star at the end
///
/// \param enabled: enabled useflag IDs
/// \param disabled: disabled useflag IDs
/// \param enforced: useflag IDs that are either forced of force_masked
/// \param changed: useflag IDs who state has changed when compared to the installed with ones
///
/// \example receives (but with flag IDs instead of their name)
///                    enabled = {"default-lld", "LLVM_TARGETS_AArch64", "LLVM_TARGETS_AMDGPU"}
///                    disabled = {"llvm-libunwind", "LLVM_TARGETS_ARC"}
///                    enforced = {"LLVM_TARGETS_AMDGPU", "LLVM_TARGETS_ARC", "LLVM_TARGETS_AArch64"}
///                    changed = {"default-lld"}
///           returns { {"LLVM_TARGETS", {"(AArch64)", "(AMDGPU)", "(-ARC)"},
///                     {"USE", {"default-lld*", "-llvm-libunwind"}} }
template <IntegerRange Range1,
          IntegerRange Range2 = std::vector<std::size_t>,
          IntegerRange Range3 = std::vector<std::size_t>,
          IntegerRange Range4 = std::vector<std::size_t>>
std::map<std::string, std::vector<std::string>>
pretty_format_flags(
        const UseFlags &useflags,
        const Range1& enabled,
        const Range2& disabled = std::vector<std::size_t>(),
        const Range3& enforced = std::vector<std::size_t>(),
        const Range4& changed = std::vector<std::size_t>())
{
    using std::unordered_set;

    auto copy_to = []<IntegerRange Range>(const Range& from, std::unordered_set<FlagID>& to )
    {
        if(not from.empty())
            std::ranges::copy(from, std::inserter(to, to.end()));
    };

    unordered_set<FlagID> enabled_set, disabled_set,
            changed_set, enforced_set;

    copy_to(enabled, enabled_set);
    copy_to(disabled, disabled_set);
    copy_to(changed, changed_set);
    copy_to(enforced, enforced_set);

    auto format_flag = [&](const unordered_set<FlagID>& flags, bool enabled, std::size_t n = 0)
    {
        ///
        /// \brief recieves a list of flag ids and formats them as enabled
        ///
        /// \param flags: list of flag IDs
        /// \param n: the number of characters to omit from the prefix of the flags
        ///           this is useful when the flags come from use expands
        /// \param enabled: is the useflag enabled
        ///
        std::map<std::string, FlagID> flag_names = useflags.to_flag_names(flags);
        std::vector<std::string> result;
        for(const auto& [flag_name, flag]: flag_names)
        {
            bool f = enforced_set.contains(flag); // is enforced ?
            bool c = changed_set.contains(flag); // is changed state ?
            result.push_back((f ? "(" : "") + fmt::format(
                                 (c ? fg(gentoo_green) : enabled ? fg(gentoo_red) : fg(gentoo_blue) ) | fmt::emphasis::bold,
                                 (enabled ? "" : "-") + flag_name.substr(n)) + (c ? "*" : "") + (f ? ")" : ""));
        }
        return result;
    };

    std::map<std::string, std::vector<std::string>> pretty_stuff;

    auto&& enabled_formatting = format_flag(enabled - useflags.get_expand_flags(), true);
    if(not enabled_formatting.empty())
        std::ranges::move(std::move(enabled_formatting), std::back_inserter(pretty_stuff["USE"]));

    auto&& disabled_formatting = format_flag(disabled - useflags.get_expand_flags(), false);
    if(not disabled_formatting.empty())
        std::ranges::move(std::move(disabled_formatting), std::back_inserter(pretty_stuff["USE"]));

    auto enabled_expanded_flags = useflags.filter_expanded_flags(enabled_set);
    auto disabled_expanded_flags = useflags.filter_expanded_flags(disabled_set);

    unordered_set<ExpandID> expand_ids;
    std::ranges::transform(enabled_expanded_flags, std::inserter(expand_ids, expand_ids.end()), [](auto& pair){return pair.first; });
    std::ranges::transform(disabled_expanded_flags, std::inserter(expand_ids, expand_ids.end()), [](auto& pair){return pair.first; });

    for(ExpandID expand_id: expand_ids)
    {
        std::string expand_name = useflags.get_expand_name_from_expand_id(expand_id);
        if(useflags.get_expand_type_from_expand_id(expand_id).hidden)
            continue;
        std::ranges::move(format_flag(enabled_expanded_flags[expand_id], true, expand_name.size()+1), std::back_inserter(pretty_stuff[expand_name]));
        std::ranges::move(format_flag(disabled_expanded_flags[expand_id], false, expand_name.size()+1), std::back_inserter(pretty_stuff[expand_name]));
    }

    return pretty_stuff;
}

/// \brief formats KEYWORD according the equery code
inline std::string format_keyword(Keywords::State keyword_state)
{
    const static std::unordered_map<Keywords::State, std::string> state_to_string =
    {
        {Keywords::State::BROKEN, fmt::format(fg(gentoo_red) | fmt::emphasis::bold, "*")},
        {Keywords::State::UNDEFINED, fmt::format(fg(gentoo_black) | fmt::emphasis::bold, "o")},
        {Keywords::State::LIVE, fmt::format(fg(gentoo_black) | fmt::emphasis::bold, "o")},
        {Keywords::State::STABLE, fmt::format(fg(gentoo_green) | fmt::emphasis::bold, "+")},
        {Keywords::State::TESTING, fmt::format(fg(gentoo_orange) | fmt::emphasis::bold, "~")},
    };

    return state_to_string.at(keyword_state);
}

inline std::string format_bool(bool bl)
{
    return bl ? fmt::format(fg(gentoo_green) | fmt::emphasis::bold, "y") : fmt::format(fg(gentoo_red) | fmt::emphasis::bold, "n");
}

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


/// \brief prints string table, which is a container of rows, each row a container of strings
/// \note  strings can be potentially formatted with ANSI escape sequences.
template <StringTable Table>
void print_table(const Table& table)
{
    // Compute bounding rects of each cell
    std::vector<size_t> column_widths, row_heights(table.size(), 1);

    {
        struct bounding_rect
        {
            size_t width=1, height=1;
        };

        auto get_bouding_rect = [](const std::string& cell_str)
        {
            bounding_rect rect;
            std::string_view view(cell_str);

            size_t newline_pos = view.find('\n');
            while(newline_pos != std::string_view::npos)
            {
                rect.height++;
                rect.width = std::max(rect.width, newline_pos);
                view.remove_prefix(newline_pos+1);
                newline_pos = view.find('\n');
            };

            rect.width = std::max(rect.width, view.size());
            return rect;
        };

        size_t i = 0;
        for(const auto& row: table)
        {
            size_t j = 0;
            column_widths.resize(std::max(row.size(), column_widths.size()), 0);
            for(const std::string& cell_str: row)
            {
                bounding_rect rect = get_bouding_rect(remove_ansi_escape(cell_str));
                column_widths[j] = std::max(column_widths[j], rect.width);
                row_heights[i] = std::max(row_heights[i], rect.height);
                j++;
            }
            i++;
        }
    }

    // Extract line from string
    auto get_line = [](const std::string& str,  size_t line_num)
    {
        size_t line = 0;
        std::string_view view(str);

        size_t newline_pos = view.find('\n');
        while(newline_pos != std::string_view::npos and line != line_num)
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
    };

    // Print rows line by line, each row may have several lines
    auto row_it = table.cbegin();
    for(size_t i = 0; i < table.size() ; i++)
    {
        const auto& row = *row_it;

        // print a row, line by line (as a cell can be multi-line
        for(size_t row_line = 0 ; row_line < row_heights[i] ; row_line++)
        {
            size_t j = 0;
            for(const std::string& cell_str: row)
            {
                std::string_view line = get_line(cell_str, row_line);
                std::string plain_line = remove_ansi_escape(std::string(line));
                size_t line_size = plain_line.size();
                assert(column_widths[j] >= line_size);
                std::cout << ' ' << line << std::string(column_widths[j] - line_size, ' ') << " |";
                j++;
            }
            std::cout << std::endl;
        }

        // print ---- to mark the end of the row

        for(size_t j = 0; j < column_widths.size() ; j++)
        {
            std::cout << std::string(column_widths[j]+2, '-') << ((j == column_widths.size() - 1) ? "|" : "+");
        }
        std::cout << std::endl;

        row_it++;
    }


}
