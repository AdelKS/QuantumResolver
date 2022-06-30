#include "quantum-resolver/cli/cli_interface.h"
#include "quantum-resolver/utils/format_utils.h"
#include "quantum-resolver/utils/string_utils.h"
#include "quantum-resolver/cli/table_print.h"
#include "quantum-resolver/utils/misc_utils.h"

CommandLineInterface::CommandLineInterface(std::vector<std::string> input)
{
    if(input.size() == 2 and input[0] == "status")
        print_pkg_status(input[1]);
}

void CommandLineInterface::print_pkg_status(const std::string &package_constraint_str)
{
    PackageConstraint pkg_constraint = db.parser.parse_pkg_constraint(package_constraint_str);
    if(pkg_constraint.pkg_id == db.repo.npos)
    {
        fmt::print("Invalid atom: {}", package_constraint_str);
        return;
    }

    auto& pkg = db.repo[pkg_constraint.pkg_id];
    std::vector<EbuildID> matched_ebuild_ids = pkg.get_matching_ebuild_ids(pkg_constraint);
    if(matched_ebuild_ids.empty())
        return;

    bool first = true;
    std::unordered_set<FlagID> shared_iuse, shared_use, shared_use_force, shared_use_mask, non_shared_flags;

    auto intersect_or_assign = [&first, &non_shared_flags](std::unordered_set<FlagID>& to, const std::unordered_set<FlagID>& from)
    {
        if(first)
            to = from;
        else
        {
            non_shared_flags += to ^ from;
            to &= from;
        }
    };

    for(auto& ebuild_id: matched_ebuild_ids)
    {
        auto& ebuild = pkg[ebuild_id];
        intersect_or_assign(shared_iuse, ebuild.get_iuse());
        intersect_or_assign(shared_use, ebuild.get_use());
        intersect_or_assign(shared_use_force, ebuild.get_use_force());
        intersect_or_assign(shared_use_mask, ebuild.get_use_mask());

        if(ebuild.is_installed())
            non_shared_flags += ebuild.get_active_flags() ^ ebuild.get_install_active_flags();

        first = false;
    }

    auto shared_use_force_strs = db.useflags.to_flag_names(shared_use_force);
    auto shared_use_mask_strs = db.useflags.to_flag_names(shared_use_mask);

    auto shared_active_flags = shared_use + shared_use_force - shared_use_mask;
    auto pretty_formatting = pretty_format_flags(
                db.useflags,
                (shared_active_flags & shared_iuse) - non_shared_flags,
                (shared_iuse - shared_active_flags) - non_shared_flags,
                (shared_use_force + shared_use_mask));

    std::cout << std::string(30, '#') << std::endl;
    fmt::print(fmt::fg(gentoo_green) | fmt::emphasis::bold, "{}", package_constraint_str);

    fmt::print(fmt::fg(gentoo_orange) | fmt::emphasis::bold, "{}",
               db.repo.is_system_pkg(pkg_constraint.pkg_id) ? "    @system" : "");

    fmt::print(fmt::emphasis::bold, "{}\n",
               db.repo.is_selected_pkg(pkg_constraint.pkg_id) ? "    @selected-packages" : "");

    std::cout << std::string(package_constraint_str.size(), '~') << std::endl << std::endl;

    const std::string shared_flag_states = matched_ebuild_ids.size() == 1 ? "Flag states" : "Shared flag states";
    std::cout << shared_flag_states << std::endl << std::string(shared_flag_states.size(), '~') << std::endl;

    for(const auto& [expand_name, flag_formatting]: pretty_formatting)
        std::cout << wrap_indent(fmt::format("{}=\"{}\"\n",
                                             expand_name,
                                             concatenate(flag_formatting, " ")),
                                 100, expand_name.size() + 3);

    std::cout << std::endl;


    const std::string versions = "Matching versions\n";
    std::cout << versions << std::string(versions.size(), '~') << std::endl << std::endl;

    std::vector<std::vector<std::string>> status_table;

    if(non_shared_flags.empty())
        status_table.push_back({"", db.useflags.get_arch_name(), "SLOT"});
    else status_table.push_back({"", db.useflags.get_arch_name(), "SLOT", "non shared flag states"});

    std::string slot;

    // we use this truct to append to strings
    enum struct Table : size_t {VERSION = 0, KEYWORD, SLOT, NON_SHARED_FLAG_STATES, SIZE};

    std::unordered_set<size_t> row_no_sep;

    for(auto& ebuild_id: matched_ebuild_ids)
    {
        auto& ebuild = pkg[ebuild_id];

        bool same_slot = ebuild.get_slot_str() == slot;

        std::vector<std::string> ebuild_line((size_t(Table::SIZE)) - non_shared_flags.empty());

        // append new lines if we remain on the same slot
        if(not same_slot)
        {
            slot = ebuild.get_slot_str();
            ebuild_line[size_t(Table::SLOT)] += fmt::format(fg(gentoo_orange) | fmt::emphasis::bold, slot);
        }
        else row_no_sep.insert(status_table.size()-1);

        if(ebuild.is_installed())
            ebuild_line[size_t(Table::VERSION)] += fmt::format(fmt::emphasis::bold, "[I] {}",  ebuild.get_version().string());
        else ebuild_line[size_t(Table::VERSION)] += fmt::format("    {}",  ebuild.get_version().string());

        ebuild_line[size_t(Table::KEYWORD)] += fmt::format("{} {}",
                                               format_keyword(ebuild.get_arch_keyword()),
                                               format_bool(ebuild.is_keyword_accepted()));

        auto ebuild_active_flags = ebuild.get_active_flags();

        if(not non_shared_flags.empty())
        {
            auto pretty_formatting = pretty_format_flags(
                        db.useflags,
                        (ebuild_active_flags & ebuild.get_iuse()) & non_shared_flags,
                        (ebuild.get_iuse() - ebuild_active_flags) & non_shared_flags,
                        ebuild.get_enforced_flags() & non_shared_flags,
                        ebuild.get_changed_flags());

            size_t index = 0;
            for(const auto& [expand_name, flag_formatting]: pretty_formatting)
            {
                bool last = index == pretty_formatting.size() - 1;
                std::string& cell = ebuild_line[size_t(Table::NON_SHARED_FLAG_STATES)];
                cell += fmt::format("{}{} += \"{}\"{}",
                                    index == 0 ? "" : "  ",
                                    expand_name,
                                    concatenate(flag_formatting, " "),
                                    last ? "" : "\n");

                index++;
            }
        }

        status_table.push_back(std::move(ebuild_line));
    }

    TablePrint table_printer;
    table_printer.set_row_sep_skip(row_no_sep);
    table_printer.set_table_max_width(100);
    table_printer.set_wrap_indent(3);
    table_printer.set_table(status_table);
    table_printer.print_table();
}
