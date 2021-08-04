#include <iostream>

using namespace std;

#include "database.h"



int main()
{
    Database database;
    try
    {
        database.populate_from_overlay("/var/db/repos/gentoo");
        database.populate_from_cache_dir("/var/db/repos/gentoo/metadata/md5-cache");
    }
    catch(string str)
    {
        cout << str << endl;
    }

    return 0;
}
