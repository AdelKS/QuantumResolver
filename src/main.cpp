#include <iostream>
#include <chrono>
using namespace std::chrono;

using namespace std;

#include "cli_interface.h"

int main(int argc, char *argv[])
{
    try
    {
        vector<string> input;
        for(int i = 1 ; i < argc ; i++)
            input.push_back(argv[i]);

//        input.push_back("status");
//        input.push_back("sys-devel/gcc");

        auto start = high_resolution_clock::now();
        CommandLineInterface cli(input);
        auto end = high_resolution_clock::now();

        cout << "#####################################################" << endl;
        cout << "Total time : " << duration_cast<milliseconds>(end - start).count() << "ms" << endl;

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
