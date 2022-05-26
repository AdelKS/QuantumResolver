#include <iostream>
#include <chrono>
using namespace std::chrono;

using namespace std;

#include "database.h"

int main(int argc, char *argv[])
{
    try
    {
        vector<string> input;
        for(int i = 0 ; i < argc ; i++)
            input.push_back(argv[i]);

        if(argc == 3 and input[1] == "status")
        {
            Database database;
            database.repo.print_flag_states(input[2]);
        }
//        Database database;
//        database.repo.print_flag_states("www-client/firefox");
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "something went wrong" << endl;
    }

    return 0;
}
