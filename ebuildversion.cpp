#include <locale>

#include "ebuildversion.h"
#include "utils.h"

const regex EbuildVersion::ver_regexp = regex("^(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r(\\d+))?$");
const vector<string> EbuildVersion::ordered_separators = {".", "_p", "_rc", "_pre", "_beta", "_alpha", "-r"};

EbuildVersion::EbuildVersion(string ver): version(ver)
{
    valid = regex_match(version, ver_regexp);

    if(valid)
    {
        vector<pair<int, string>> split = split_string(version, ordered_separators);

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

            letter_found = couple.first == 0 && isalpha(couple.second.back(), locale("C"));
            if(letter_found)
            {
                letter_number = couple.second.back();
                couple.second = couple.second.substr(0, couple.second.size()-1);
            }

            number = stol(couple.second, &processed_chars);
            if(processed_chars == couple.second.size())
            {
              version_parsing.emplace_back(couple.first, number);
              if(letter_found)
                  version_parsing.emplace_back(0, letter_number);
            }
            else throw "the following string couldn't be converted entirely to an integer: " + couple.second;

        }
    }
    else
    {
        throw "Version " + version + " is of invalid format";
    }
}
