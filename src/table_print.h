#ifndef TABLEPRINT_H
#define TABLEPRINT_H

#include "concepts.h"

#include <bits/ranges_algo.h>
#include <bits/ranges_base.h>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <ranges>
#include <numeric>

class TablePrint
{
public:
    TablePrint();

    /// \brief skips row separation after each row in 'row_no_sep'
    template <std::ranges::common_range Range>
    requires std::is_same_v<std::ranges::range_value_t<Range>, size_t>
    void set_row_sep_skip(const Range& row_no_sep)
    {
        std::ranges::copy(row_no_sep, std::inserter(row_sep_skip, row_sep_skip.end()));
    }

    /// \brief sets the table max width
    /// \note  table is build with columns that fit the width of their cells
    ///        if the result is a table wider than max width, the longest cells
    ///        get wrapped at spaces to reduce their width
    void set_table_max_width(size_t width);

    /// \brief sets the indent size for wrapped new lines, 0 by default
    void set_wrap_indent(size_t width);

    /// \brief sets the table to print, wraps cells before saving in internal variable
    /// \param table: table to print, it's a container of a container of strings
    /// \note  strings can be potentially formatted with ANSI escape sequences.
    template <StringTable Table>
    void set_table(const Table& rows)
    {
        table.clear();
        for(const auto& row: rows)
        {
            table.push_back(std::vector<std::string>());
            for(const std::string& cell_str: row)
                table.back().push_back(cell_str);
        }

        compute_widths();
        if(max_table_width != 0)
        {
            compute_column_wraps();
            wrap_columns();
        }
    }

    /// \brief prints the table
    void print_table();

protected:
    void compute_widths();
    void compute_column_wraps();
    void wrap_columns();

    std::size_t max_table_width = 0, wrap_indent_size = 0;
    std::unordered_set<size_t> wrapped_cols;
    std::vector<std::vector<std::string>> table; // list of rows
    std::vector<size_t> min_column_widths, column_widths, row_heights;
    std::unordered_set<std::size_t> row_sep_skip;
};

#endif // TABLEPRINT_H
