#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "seashell.h"


static void print_usage(const char *binary_name)
{
    printf("Usage: %s [params]\n", binary_name);
    printf("Available params:\n");
    printf(" -h, --help     : Display this help and exit\n");
    printf(" -v, --version  : Show version information\n");
    printf(" -f, --features : Show seashell features\n");
}


static void print_features()
{
    printf("\n=== S E A S H E L L ===\n");
    printf("\nFeatures List:\n");
    printf("\n=INTERFACE MANAGEMENT=\n");
    printf("cursor motion:DONE!\n");
    printf("- cursor left:done\n");
    printf("- cursor right:done\n");
    printf("buffer management:DONE!\n");
    printf("- insert_char:done\n");
    printf("- remove_char:done\n");
    printf("autocompletion:TODO!\n");
    printf("-tab key management:todo\n");
    printf("jobcontrols:TODO!\n");
    printf("history: TODO!\n");
    printf("- cursor up: done\n");
    printf("- cursor down: done\n");
    printf("- don't fill list if same last entry: todo\n");
    printf("- implement history rotation with a limited size: todo\n");
    printf("- history based on .history file for persistency: todo\n");
    printf("\n=EXECUTION MANAGEMENT=\n");
    printf("bultins:TODO!\n");
    printf("- cd:todo\n");
    printf("- echo:todo\n");
    printf("- exit:todo\n");
    printf("redirection:TODO!\n");
    printf("- >:todo\n");
    printf("- >>:todo\n");
    printf("- <:todo\n");
    printf("multipipes:DONE!\n");
    printf("globbing:DONE!\n");
    printf("\n=CONFIGURATION MANAGEMENT=\n");
    printf("- prompt management:todo\n");
    printf("- path to history:todo\n");
    printf("- aliases:todo\n");
}


static void print_version()
{
    printf("seashell, version \?.\?.\?\?(\?)-release\n");
    printf("Linux C shell - GNU bash mimic\n");
    printf("Copyright (C) 2017 pestbuns\n");
    printf("MIT License: <https://opensource.org/licenses/MIT>\n");
    printf("\n");
    printf("This is free software; you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}


int option_manager(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help",        no_argument, 0, 'h'},
        {"version",     no_argument, 0, 'v'},
        {"features",    no_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    const char *option_string = "hvf";
    int c;
    while ((c = getopt_long(argc, argv, option_string, long_options, NULL)) != -1) {
        switch (c) {
            case 'h': print_usage(argv[0]); exit(EXIT_SUCCESS);
            case 'v': print_version(); exit(EXIT_SUCCESS);
            case 'f': print_features(); exit(EXIT_SUCCESS);
            default: print_usage(argv[0]); exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
