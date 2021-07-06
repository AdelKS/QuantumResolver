#include "ebuildversion.h"
#include "utils.h"

const regex EbuildVersion::ver_regexp = regex("^(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r(\\d+))?$");
const vector<string> EbuildVersion::ordered_separators = {".", "_alpha", "_beta", "_pre", "_rc", "_p", "-r"};

EbuildVersion::EbuildVersion(string ver): version(ver)
{
    smatch match;
    valid = regex_match(version, match, ver_regexp);

    if(valid)
    {
        vector<pair<string, string>> split = split_string(version, ordered_separators);
    }
    else
    {
        cout << "Version " << version << " is of invalid format" << endl;
    }
}
