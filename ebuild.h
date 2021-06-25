#ifndef EBUILD_H
#define EBUILD_H

#include <set>
#include <string>
#include <map>

#include "stringindexedvector.h"

using namespace std;

class Ebuild
{
public:
    Ebuild(string ebuild_ver);

protected:
    string ver;
    set<string> avail_use_flags;
};

#endif // EBUILD_H
