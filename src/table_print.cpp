#include "table_print.h"
#include "format_utils.h"
#include "string_utils.h"
#include <cstddef>

using namespace std;

TablePrint::TablePrint()
{

}

void TablePrint::compute_widths()
{
    // Compute bounding rects of each cell

    column_widths.clear();
    row_heights.clear();
    min_column_widths.clear();
    row_heights.resize(table.size(), 1);

    size_t i = 0;
    for(const auto& row: table)
    {
        column_widths.resize(std::max(row.size(), column_widths.size()), 0);
        min_column_widths.resize(std::max(row.size(), column_widths.size()), 0);

        size_t j = 0;
        for(const std::string& cell_str: row)
        {
            StringBoundingRect rect = get_bouding_rect(cell_str);
            min_column_widths[j] = std::max(min_column_widths[j], get_min_width(cell_str));
            column_widths[j] = std::max(column_widths[j], rect.width);
            row_heights[i] = std::max(row_heights[i], rect.height);
            j++;
        }
        i++;
    }

}

void TablePrint::compute_column_wraps()
{
    auto is_wrappable = [&](size_t index)
    {
        return column_widths[index] > min_column_widths[index];
    };

    wrapped_cols.clear();

    // compute wrapping if the table is too big
    size_t current_width = std::accumulate(column_widths.begin(), column_widths.end(), table.size()*2);
    if(max_table_width != 0 and current_width > max_table_width)
    {
        if(std::accumulate(min_column_widths.begin(), min_column_widths.end(), table.size()*2) > max_table_width)
            throw std::runtime_error("table cannot satisfy width constraints");

        // we need widest and second_widest to be
        // *widest > *second_widest (notice the strict relationship
        size_t *widest = nullptr, *second_widest = nullptr;
        while(current_width > max_table_width)
        {
            for(size_t col = 0 ; col < column_widths.size() ; col++)
            {
                if(not is_wrappable(col))
                    continue;

                if(widest == nullptr)
                {
                    assert(second_widest == nullptr);
                    widest = &column_widths[col];
                    continue;
                }

                if(column_widths[col] >= *widest)
                {
                    second_widest = widest;
                    widest = &column_widths[col];
                }
                else if(column_widths[col] > *second_widest)
                    second_widest = &column_widths[col];
            }

            // this shouldn't happen as we should've thrown o
            assert(widest != nullptr);

            size_t widest_index = size_t(widest - &column_widths[0]);
            size_t wrap_amount = current_width - max_table_width;
            if(second_widest != nullptr) [[likely]]
            {
                if(*widest != *second_widest)
                {
                    *widest = std::ranges::max(array{min_column_widths[widest_index], *second_widest,
                                               (*widest > wrap_amount) * (*widest - wrap_amount)});
                }
                else
                {
                    *widest -= wrap_amount/column_widths.size() + 1;
                    *second_widest -= wrap_amount/column_widths.size() + 1;
                }
            }
            else [[unlikely]]
            {
                // this is the only column that can be wrapped
                assert(*widest - wrap_amount >= min_column_widths[widest_index]);
                *widest -= wrap_amount;
            }

            wrapped_cols.insert(widest_index);

            current_width = std::accumulate(column_widths.begin(), column_widths.end(), size_t(0));
        }
    }
}

void TablePrint::set_table_max_width(size_t width)
{
    max_table_width = width;
    if(not table.empty())
    {
        compute_widths();
        compute_column_wraps();
        wrap_columns();
    }
}

void TablePrint::set_wrap_indent(size_t width)
{
    wrap_indent_size = width;
}

void TablePrint::wrap_columns()
{
    // save the table and wrap if necessary
    size_t i = 0;
    for(auto& row: table)
    {
        size_t j = 0;
        for(std::string& cell_str: row)
        {
            if(wrapped_cols.contains(j))
                cell_str = wrap_indent(cell_str, column_widths[j], wrap_indent_size);
            j++;
        }
        i++;
    }
    compute_widths();
}

void TablePrint::print_table()
{
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

        row_it++;

        // print ---- to mark the end of the row
        if(row_sep_skip.contains(i))
            continue;

        for(size_t j = 0; j < column_widths.size() ; j++)
        {
            std::cout << std::string(column_widths[j]+2, '-') << ((j == column_widths.size() - 1) ? "|" : "+");
        }
        std::cout << std::endl;

    }
}
