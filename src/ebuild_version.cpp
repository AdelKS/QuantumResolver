#include <locale>
#include <algorithm>
#include <iostream>

#include "ebuild_version.h"
#include "misc_utils.h"

using namespace std;

template<class Object>
size_t index_of(const vector<Object> vec, Object obj)
{
    // Get the index of obj in vec
    return find(vec.cbegin(), vec.cend(), obj) - vec.cbegin();
}

// The regex that validates EBUILD version strings
const regex EbuildVersion::ver_regexp = regex("^(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r(\\d+))?$");

// All the separators in an EBUILD string
const vector<string> EbuildVersion::ordered_separators = {"-r", "_alpha", "_beta", "_pre", "_rc", "_p", "."};

// Index of "." in ordered_separators
const size_t EbuildVersion::dot_index = index_of(EbuildVersion::ordered_separators, string("."));


inline bool is_smaller_than_nothing(const size_t sep_index)
{
    // if sep_index refers to "_rc", "_pre", "_beta" or "_alpha", return true.
    // If an ebuild has a version string v, then v' = v + (any of the separators above) + (whatever else) makes the version v' smaller
    // Example "1.0_alpha" < "1.0", "1.0_rc3234" < "1.0"

    return 1 <= sep_index and sep_index <= 4;
}

EbuildVersion::EbuildVersion(string ver)
{
    set_version(ver);
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

    throw runtime_error("Version constraint check failed.");
}

const std::string &EbuildVersion::get_version()
{
    return version;
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
        throw runtime_error("Version string of invalid format : " + ver);

    version = ver;
    vector<pair<size_t, string_view>> split = split_string(ver, ordered_separators, dot_index);

    long letter_number = 0, number = 0;
    char *end_char;
    bool letter_found = false;

    // convert split to version_parsing
    for(auto &[index, str]: split)
    {
        if(str.empty())
        {
            version_parsing.emplace_back(index, 0);
            continue;
        }

        letter_found = isalpha(str.back(), locale("C"));
        if(letter_found)
        {
            if(index != dot_index)
                throw runtime_error("something is wrong with this version string: " + ver);

            letter_number = str.back();
            str.remove_suffix(1);
        }

        number = strtol(str.data(), &end_char, 10);
        if(size_t(end_char - str.data()) == str.size())
        {
          version_parsing.emplace_back(index, number);
          if(letter_found)
              version_parsing.emplace_back(dot_index, letter_number);
        }
        else throw runtime_error("the following string couldn't be converted entirely to an integer: " + string(str));

    }
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

    const size_t min_size = min(a.version_parsing.size(), b.version_parsing.size());
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + min_size,
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + min_size);

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

    const size_t min_size = min(a.version_parsing.size(), b.version_parsing.size());
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + min_size,
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + min_size);

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

    const size_t min_size = b.version_parsing.size();
    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + min_size,
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + min_size);

    return res == 0;

}

bool operator ^= (const EbuildVersion &a, const EbuildVersion &b)
{
    // returns true if this ~ 1.23

    size_t rev_sep_index = index_of(a.ordered_separators, string("-r"));

    // we compare packages without revision numbers. e.g. [(".", 1), (".", 2), ("-r", 1)] -> [(".", 1), (".", 2)]
    const size_t size = a.version_parsing.back().first == rev_sep_index ? a.version_parsing.size() - 1 : a.version_parsing.size();
    const size_t other_size = b.version_parsing.back().first == rev_sep_index ? b.version_parsing.size() - 1 : b.version_parsing.size();

    // when the revision numbers are omitted, the version parsings should have the same length
    if(size != other_size)
        return false;

    const auto res = lexicographical_compare_three_way(a.version_parsing.cbegin(),  a.version_parsing.cbegin() + size,
                                                       b.version_parsing.cbegin(),  b.version_parsing.cbegin() + size);

    return res == 0;

}
