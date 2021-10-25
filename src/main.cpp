#include <iostream>

using namespace std;

#include "database.h"



int main(int argc, char *argv[])
{
    Database database;
    try
    {
        database.populate_from_cache_dir("/var/db/repos/gentoo/metadata/md5-cache");
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }

    return 0;
}
