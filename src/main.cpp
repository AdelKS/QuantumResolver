#include <iostream>

using namespace std;

#include "database.h"



int main(int argc, char *argv[])
{
    Database database;
    try
    {
        database.populate("/var/db/repos/gentoo/metadata/md5-cache");
        cout << "Done populating!" << endl;
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }

    return 0;
}
