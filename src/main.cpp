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
        database.print_flag_states("www-client/firefox");
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }

    return 0;
}
