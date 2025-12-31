// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A2519B6

#include <vector>
#include <iostream>
#include <algorithm>
#include <getopt.h>
#include <string>

#include "coreHeaders.hpp"

// ----------------------------------------------------------------------------
//                               Driver
// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{

    try
    {
        std::ios_base::sync_with_stdio(false);

        Market marketObject;

        marketObject.get_options(argc, argv);

        marketObject.readInput();

        marketObject.runOutput();
    }

    // Catch runtime_errors, print the message, and exit the
    // program with a non-zero status.
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // throw std::runtime_error("No print argument was provided!"

    // All done!
    return 0;
}
