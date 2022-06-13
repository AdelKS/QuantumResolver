#pragma once

#include "concepts.h"
#include "useflags.h"

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
    ///
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
    ///

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