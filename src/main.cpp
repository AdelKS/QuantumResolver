#include <iostream>

using namespace std;

#include "database.h"



int main(int argc, char *argv[])
{
    Database database;
    try
    {
        cout << argc << endl;
        database.populate("/var/db/repos/gentoo/metadata/md5-cache");
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }

    return 0;
}
