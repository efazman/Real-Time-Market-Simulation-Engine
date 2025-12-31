// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A2519B6

#include <getopt.h>
#include "coreHeaders.hpp"

void Market::get_options(int argc, char **argv)
{
    struct option longOpts[] = {
        {"verbose", no_argument, NULL, 'v'},
        {"median", no_argument, NULL, 'm'},
        {"trader_info", no_argument, NULL, 'i'},
        {"time_travelers", no_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
    };

    // Turn getopt error message on (true) or off (false)
    // opterr is declared in getopt.h, is on by default
    opterr = false;

    int opt = 0, index = 0;
    // "ab:h" repeats the allowed flags
    // The : after b dontes the required argument for option b
    while ((opt = getopt_long(argc, argv, "hvmit", longOpts, &index)) != -1)
    {
        switch (opt)
        {
        case 'v':
            v = 1;
            break;
        case 'm':
            // optarg is defined in getopt.h
            m = 1;
            break;
        case 'i':
            // optarg is defined in getopt.h
            i = 1;
            break;
        case 't':
            // optarg is defined in getopt.h
            t = 1;
            break;
        case 'h':
            cout << "You want help?  This program accepts:\n";
            cout << "  -a, --choiceA       The first choice\n";
            cout << "  -b, --choiceB ARG   This choice needs an argument\n";
            cout << "  -h, --help          This help page\n";
            exit(0);
        } // switch
    } // while

    
}