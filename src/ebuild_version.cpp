#include <locale>
#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>

#include "ebuild_version.h"
#include "misc_utils.h"

template<class Object>
std::size_t index_of(const std::vector<Object> vec, Object obj)
{
    // Get the index of obj in vec
    return std::size_t(find(vec.cbegin(), vec.cend(), obj) - vec.cbegin());
}

// The regex that validates EBUILD version strings
const std::regex EbuildVersion::ver_regexp = std::regex("^(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r(\\d+))?$");

// All the separators in an EBUILD string
const std::vector<std::string> EbuildVersion::ordered_separators = {"-r", "_alpha", "_beta", "_pre", "_rc", "_p", "."};

// Index of "." in ordered_separators
const std::size_t EbuildVersion::dot_index = index_of(EbuildVersion::ordered_separators, std::string("."));


inline bool is_smaller_than_nothing(const std::size_t sep_index)
{
    // if sep_index refers to "_rc", "_pre", "_beta" or "_alpha", return true.
    // If an ebuild has a version string v, then v' = v + (any of the separators above) + (whatever else) makes the version v' smaller
    // Example "1.0_alpha" < "1.0", "1.0_rc3234" < "1.0"

    return 1 <= sep_index and sep_index <= 4;
}

EbuildVersion::EbuildVersion() : live(false)
{
}

EbuildVersion::EbuildVersion(std::string ver) : live(false)
{
    set_version(std::move(ver));
}

bool EbuildVersion::respects_constraint(const VersionConstraint &constraint)
{
    switch (constraint.type) {
    case VersionConstraint::Type::NONE:
        return true;
        break;
    case VersionConstraint::Type::SLESS:
        return *this < constraint.version;
        break;
    case VersionConstraint::Type::LESS:
        return *this <= constraint.version;
        break;
    case VersionConstraint::Type::EQ_REV:
        return *this ^= constraint.version;
        break;
    case VersionConstraint::Type::EQ_STAR:
        return *this *= constraint.version;
        break;
    case VersionConstraint::Type::EQ:
        return *this == constraint.version;
        break;
    case VersionConstraint::Type::GREATER:
        return *this >= constraint.version;
        break;
    case VersionConstraint::Type::SGREATER:
        return *this > constraint.version;
        break;
    }

    throw std::runtime_error("Version constraint check failed.");
}

const std::string &EbuildVersion::string() const
{
    return version;
}

bool EbuildVersion::is_live()
{
    return live;
}

void EbuildVersion::update_live_bool(const std::vector<std::pair<std::size_t, std::string_view>>& split)
{
    /// Update the bool 'live' that says if this version is a live version
    auto res = std::ranges::find_if(split.rbegin(), split.rend(), [](const auto& pair){return pair.first == dot_index;});
    live = res != split.rend() and res->second.starts_with("9999");
}

void EbuildVersion::set_version(std::string ver)
{
    if(ver.empty())
    {
        version.clear();
        version_parsing.clear();
        return;
    }

    if(not regex_match(ver, ver_regexp))
        throw std::runtime_error("Version string of invalid format : " + ver);

    std::vector<std::pair<std::size_t, std::string_view>> split = split_string(ver, ordered_separators, dot_index);

    // check the string split for "9999" to know if the ebuild is a live one
    update_live_bool(split);

    long letter_number = 0;
    ulong number = 0;
    std::size_t end_pos;
    bool letter_found = false;

    // convert split to version_parsing
    for(auto &[index, str_view]: split)
    {
        if(str_view.empty())
        {
            version_parsing.emplace_back(index, 0);
            continue;
        }

        letter_found = isalpha(str_view.back(), std::locale("C"));
        if(letter_found)
        {
            if(index != dot_index)
                throw std::runtime_error("something is wrong with this version string: " + ver);

            letter_number = str_view.back();
            str_view.remove_suffix(1);
        }

        // one could use `strtoul` wiht str_view.data() but we cannot limit where to stop.
        number = stoul(std::string(str_view), &end_pos);
        if(end_pos == str_view.size())
        {
          version_parsing.emplace_back(index, number);
          if(letter_found)
              version_parsing.emplace_back(dot_index, letter_number);
        }
        else throw std::runtime_error("the following string couldn't be converted entirely to an integer: " + std::string(str_view));

    }

    version = std::move(ver);
}


bool operator < (const EbuildVersion &a, const EbuildVersion &b)
{
    /* returns true if this < 1.23
     * we lexicographically compare the two version_parsing variables at same size. If they are equal, we check the next subversion in the longer one to know if it's newer
     *  examples:
     *      [(".", 1), (".", 2)] < [(".", 1), (".", 3)] aka     1.2 < 1.3
     *      [(".", 1)] < [(".", 1), ("_p", 1)]          aka     1 < 1_p1
     *      [(".", 1)] < [(".", 1), ("-r", 1)]          aka     1 < 1-r1
     *      [(".", 1), ("_rc", 1)] < [(".", 1)]         aka     1_rc1 < 1
     *      Note: "-r", ".", "_rc" are actually referenced by their integer priority, given by their index in ordered_separators
     *      */

    const std::size_t min_size = std::min(a.version_parsing.size(), b.version_parsing.size());
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + long(min_size),
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + long(min_size));

    bool result = res < 0 or (
                            res == 0 and (
                                    (a.version_parsing.size() > min_size and is_smaller_than_nothing(a.version_parsing[min_size].first)) or
                                    (b.version_parsing.size() > min_size and not is_smaller_than_nothing(b.version_parsing[min_size].first))
                            ));

    return result;
}

bool operator <= (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this <= 1.23

    const std::size_t min_size = std::min(a.version_parsing.size(), b.version_parsing.size());
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + long(min_size),
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + long(min_size));

    bool result = res <= 0 or (
                            res == 0 and (
                                    (a.version_parsing.size() > min_size and is_smaller_than_nothing(a.version_parsing[min_size].first)) or
                                    (b.version_parsing.size() > min_size and not is_smaller_than_nothing(b.version_parsing[min_size].first))
                            ));

    return result;
}

bool operator >  (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this > 1.23

    return not (a <= b);

}

bool operator >= (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this >= 1.23

    return not (a < b);
}

bool operator == (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this = 1.23

    if(a.version_parsing.size() != b.version_parsing.size())
        return false;

    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cend(),
                                                       b.version_parsing.cbegin(),  b.version_parsing.cend());

    return res == 0;

}

bool operator *= (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this matches with =1.23*

    // 1.2 does not match with =1.2.3*
    if(a.version_parsing.size() < b.version_parsing.size())
        return false;

    const std::size_t min_size = b.version_parsing.size();
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + long(min_size),
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + long(min_size));

    return res == 0;

}

bool operator ^= (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this ~ 1.23

    std::size_t rev_sep_index = index_of(a.ordered_separators, std::string("-r"));

    // we compare packages without revision numbers. e.g. [(".", 1), (".", 2), ("-r", 1)] -> [(".", 1), (".", 2)]
    const std::size_t size = a.version_parsing.back().first == rev_sep_index ? a.version_parsing.size() - 1 : a.version_parsing.size();
    const std::size_t other_size = b.version_parsing.back().first == rev_sep_index ? b.version_parsing.size() - 1 : b.version_parsing.size();

    // when the revision numbers are omitted, the version parsings should have the same length
    if(size != other_size)
        return false;

    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + long(size),
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + long(size));

    return res == 0;

}
