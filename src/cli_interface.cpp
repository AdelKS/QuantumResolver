#include "cli_interface.h"
#include "src/format_utils.h"
#include <fmt/color.h>
#include <fmt/format.h>

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
        fmt::print("{}=\"{}\"\n", expand_name, fmt::join(flag_formatting, " "));

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

    for(auto& ebuild_id: matched_ebuild_ids)
    {
        auto& ebuild = pkg[ebuild_id];

        bool same_slot = ebuild.get_slot_str() == slot;

        std::vector<std::string> new_ebuild_line((size_t(Table::SIZE)) - non_shared_flags.empty());

        std::vector<std::string>& ebuild_line = same_slot ? status_table.back() : new_ebuild_line;

        auto append_newline = [&]()
        {
            for(auto& str: ebuild_line)
                str += '\n';
        };

        // append new lines if we remain on the same slot
        if(same_slot)
            append_newline();
        else
        {
            slot = ebuild.get_slot_str();
            ebuild_line[size_t(Table::SLOT)] += fmt::format(fg(gentoo_orange) | fmt::emphasis::bold, slot);
        }


        if(ebuild.is_installed())
            ebuild_line[size_t(Table::VERSION)] += fmt::format(fmt::emphasis::bold, "[I] {}",  ebuild.get_version().string());
        else ebuild_line[size_t(Table::VERSION)] += fmt::format("    {} ",  ebuild.get_version().string());

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
            const std::string flags_padding(3, ' ');
            for(const auto& [expand_name, flag_formatting]: pretty_formatting)
            {
                const bool last = index == pretty_formatting.size() - 1;
                std::string& cell = ebuild_line[size_t(Table::NON_SHARED_FLAG_STATES)];
                const auto& flags_split  = split_string_list(flag_formatting, 40);

                const std::string padding((index != 0) * 2, ' ');

                cell += padding + expand_name + " += ";

                if(flags_split.size() == 1)
                    cell += fmt::format("\"{}\"", fmt::join(flags_split.front(), " "));
                else
                {
                    append_newline();
                    for(size_t split_index = 0 ; split_index < flags_split.size() ; split_index++)
                    {
                        cell += padding + flags_padding + (split_index == 0 ? "\"" : " ") +
                                fmt::format("{}", fmt::join(flags_split[split_index], " "));
                        if(split_index != flags_split.size()-1)
                            append_newline();
                    }
                    cell += "\"";
                }

                if(not last)
                    append_newline();

                index++;
            }
        }

        if(not same_slot)
            status_table.push_back(std::move(new_ebuild_line));
    }

    print_table(status_table);
}
