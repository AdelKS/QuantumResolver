#include "cli_interface.h"

CommandLineInterface::CommandLineInterface(std::vector<std::string> input)
{
    if(input.size() == 2 and input[0] == "status")
        print_flag_states(input[1]);
}

void CommandLineInterface::print_flag_states(const std::string &package_constraint_str)
{
    PackageConstraint pkg_constraint = db.parser.parse_pkg_constraint(package_constraint_str);
    if(pkg_constraint.pkg_id == db.repo.npos)
    {
        fmt::print("Invalid atom: {}", package_constraint_str);
        return;
    }

    std::vector<EbuildID> matched_ebuild_ids = db.repo[pkg_constraint.pkg_id].get_matching_ebuild_ids(pkg_constraint);
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
        auto& ebuild = db.repo[pkg_constraint.pkg_id][ebuild_id];
        intersect_or_assign(shared_iuse, ebuild.get_iuse());
        intersect_or_assign(shared_use, ebuild.get_use());
        intersect_or_assign(shared_use_force, ebuild.get_use_force());
        intersect_or_assign(shared_use_mask, ebuild.get_use_mask());

        if(ebuild.is_installed())
            non_shared_flags += ebuild.get_active_flags() ^ ebuild.get_install_active_flags();

        first = false;
    }

//        auto shared_use_force_strs = db.useflags.to_flag_names(shared_use_force);
//        auto shared_use_mask_strs = db.useflags.to_flag_names(shared_use_mask);

    auto shared_active_flags = shared_use + shared_use_force - shared_use_mask;
    auto pretty_formatting = pretty_format_flags(
                db.useflags,
                (shared_active_flags & shared_iuse) - non_shared_flags,
                (shared_iuse - shared_active_flags) - non_shared_flags,
                (shared_use_force + shared_use_mask));

    std::cout << std::string(30, '#') << std::endl;
    fmt::print(fmt::fg(gentoo_green) | fmt::emphasis::bold, "{}\n", package_constraint_str);

    std::cout << std::string(20, '~') << std::endl;
    std::cout << "Shared states" << std::endl << std::string(20, '-') << std::endl;

    for(const auto& [expand_name, flag_formatting]: pretty_formatting)
        fmt::print("{}=\"{}\"\n", expand_name, fmt::join(flag_formatting, " "));

    std::cout << std::string(20, '~') << std::endl;
    fmt::print("Matching versions\n", db.repo[pkg_constraint.pkg_id].get_pkg_groupname());

    for(auto& ebuild_id: matched_ebuild_ids)
    {
        auto& ebuild = db.repo[pkg_constraint.pkg_id][ebuild_id];
        std::cout << std::string(20, '-') << std::endl;

        if(ebuild.is_installed())
            fmt::print(fmt::emphasis::bold, "[I] {} ",  ebuild.get_version().string());
        else fmt::print("    {} ",  ebuild.get_version().string());

        auto ebuild_active_flags = ebuild.get_active_flags();

        auto pretty_formatting = pretty_format_flags(
                    db.useflags,
                    (ebuild_active_flags & ebuild.get_iuse()) & non_shared_flags,
                    (ebuild.get_iuse() - ebuild_active_flags) & non_shared_flags,
                    ebuild.get_enforced_flags() & non_shared_flags,
                    ebuild.get_changed_flags());

        for(const auto& [expand_name, flag_formatting]: pretty_formatting)
            fmt::print(" {}=\"{}\"", expand_name, fmt::join(flag_formatting, " "));

        std::cout << std::endl;
    }

    std::cout << std::string(20, '-') << std::endl;
}
