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
    Ebuild(int ebuild_id, string ebuild_ver);

protected:
    int id;
    string ver;
    set<string> avail_use_flags;
};

#endif // EBUILD_H
