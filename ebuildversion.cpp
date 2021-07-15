#include <locale>
#include <algorithm>

#include "ebuildversion.h"
#include "utils.h"

template<class Object>
int index_of(const vector<Object> vec, Object obj)
{
    // Get the index of obj in vec
    return find(vec.cbegin(), vec.cend(), obj) - vec.cbegin();
}

// The regex that validates EBUILD version strings
const regex EbuildVersion::ver_regexp = regex("^(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r(\\d+))?$");

// All the separators in an EBUILD string
const vector<string> EbuildVersion::ordered_separators = {"-r", "_alpha", "_beta", "_pre", "_rc", "_p", "."};

// Index of "." in ordered_separators
const int EbuildVersion::dot_index = index_of(EbuildVersion::ordered_separators, string("."));

// Index of the separators that make an EBUILD's version smaller that another EBUILD who is stricly equal but has these separators appended
// Example "1.0_alpha" < "1.0", "1.0_rc3234" < "1.0"
// It's "_alpha", "_beta", "_pre", and "_rc"
const unordered_set<int> EbuildVersion::smaller_than_nothing_separators =
{
    index_of(EbuildVersion::ordered_separators, string("_rc")),
    index_of(EbuildVersion::ordered_separators, string("_pre")),
    index_of(EbuildVersion::ordered_separators, string("_beta")),
    index_of(EbuildVersion::ordered_separators, string("_alpha"))
};

EbuildVersion::EbuildVersion(string ver): version(ver)
{
    valid = regex_match(version, ver_regexp);

    if(valid)
    {
        vector<pair<int, string>> split = split_string(version, ordered_separators, dot_index);

        long letter_number = 0, number = 0;
        unsigned long processed_chars = 0;
        bool letter_found = false;

        // convert split to version_parsing
        for(pair<int, string> &couple: split)
        {
            if(couple.second.empty())
            {
                version_parsing.emplace_back(couple.first, 0);
                continue;
            }

            letter_found = isalpha(couple.second.back(), locale("C"));
            if(letter_found)
            {
                if(couple.first != dot_index)
                    throw "something is wrong with this version string: " + version;

                letter_number = couple.second.back();
                couple.second = couple.second.substr(0, couple.second.size()-1);
            }

            number = stol(couple.second, &processed_chars);
            if(processed_chars == couple.second.size())
            {
              version_parsing.emplace_back(couple.first, number);
              if(letter_found)
                  version_parsing.emplace_back(dot_index, letter_number);
            }
            else throw "the following string couldn't be converted entirely to an integer: " + couple.second;

        }
    }
    else
    {
        throw "Version string of invalid format : " + version;
    }
}

bool EbuildVersion::operator < (const EbuildVersion &other)
{
    /* we lexicographically compare the two version_parsing variables at same size. If they are equal, we check the next subversion in the longer one to know if it's newer
     *  examples:
     *      [(".", 1), (".", 2)] < [(".", 1), (".", 3)] aka     1.2 < 1.3
     *      [(".", 1)] < [(".", 1), ("_p", 1)]          aka     1 < 1_p1
     *      [(".", 1)] < [(".", 1), ("-r", 1)]          aka     1 < 1-r1
     *      [(".", 1), ("_rc", 1)] < [(".", 1)]         aka     1_rc1 < 1
     *      Note: "-r", ".", "_rc" are actually referenced by their integer priority, given by their index in ordered_separators
     *      */

    const size_t min_size = min(version_parsing.size(), other.version_parsing.size());
    const auto res = lexicographical_compare_three_way(version_parsing.cbegin(),        version_parsing.cbegin() + min_size,
                                                       other.version_parsing.cbegin(),  other.version_parsing.cbegin() + min_size);

    bool result = res == strong_ordering::less or (
                            res == strong_ordering::equal and (
                                    (version_parsing.size() > min_size and smaller_than_nothing_separators.contains(version_parsing[min_size].first)) or
                                    (other.version_parsing.size() > min_size and not smaller_than_nothing_separators.contains(other.version_parsing[min_size].first))
                            ));

    if(res == strong_ordering::equal and (version_parsing.size() > min_size or other.version_parsing.size() > min_size))
    {
        cout << "Found our case!" << endl;
    }

    return result;
}
