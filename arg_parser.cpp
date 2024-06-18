#include <getopt.h>
#include <stdlib.h>

#include "arg_parser.hpp"

static void show_help() {
    printf ("--help, -h\n"
            "\tPrint this message and exit\n"
            "\n"
            "--directory, -d\n"
            "\tPath to the directory demon look after\n"
            "\n"
            "--periode, -p\n"
            "\tPeriode in sec for the demon\n");
}

static void parse_args (int argc, char **argv) {
    int res = 0, ind = 0;
    const char *short_opts = "hf:";
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"directory", required_argument, NULL, 'd'},
        {"periode", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

    while ((res = getopt_long(argc, argv, short_opts, long_opts, &ind)) != -1)
        switch (res)
        {
            default:
            case 'h': {
                show_help();
                return;
            }
            case 'd': {
                path_to_directory = optarg;
                break;
            }
            case 'p': {
                periode = atoi(optarg);
                break;
            }
        }
}