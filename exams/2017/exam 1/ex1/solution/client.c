#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "client.h"

/** default port number */
#define DEFAULT_PORTNUM     (2017)
#define DEFAULT_PORTNUM_STR "2017"

/** name of the executable (for printing messages) */
char *program_name = "<not yet set>";

int main(int argc, char **argv)
{
    struct args arguments =
        { DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0 };

    /* set program_name */
    if (argc > 0) {
        program_name = argv[0];
    }

    /***********************************************************************
     * Task 1
     * ------
     * Implement argument parsing for the client. Synopsis:
     *   ./client [-p PORT] {-g|-s VALUE} ID
     *
     * Call usage() if invalid options or arguments are given (there is no
     * need to print a description of the problem).
     *
     * Hints: getopt(3), UINT16_MAX, parse_number (client.h),
     *        struct args (client.h)
     ***********************************************************************/

    /* COMPLETE AND EXTEND THE FOLLOWING CODE */
    int c;
    int opt_p = 0;
    long num;

    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch (c) {
        case 'p':
            if (opt_p != 0)
                usage();

            num = parse_number(optarg);
            if (num < 1024 || num > UINT16_MAX)
                usage();

            arguments.portnum = (uint16_t) num;
            arguments.portstr = optarg;
            opt_p = 1;
            break;

        }
    }

    /* DO NOT FORGET ABOUT THE POSITIONAL ARGUMENTS */


    /* DO NOT REMOVE THE FOLLOWING LINE */
    apply_command(arguments);

    return 0;
}