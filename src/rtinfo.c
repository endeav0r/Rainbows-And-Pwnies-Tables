#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("rtmerge <chains>\n");
}

int main (int argc, char * argv[])
{
    _chains * chains;

    int c;
    char *   filename = NULL;

    while ((c = getopt(argc, argv, "h")) != -1) {
        switch (c) {
        case 'h' :
            print_help();
            return 0;
        case '?' :
            fprintf(stderr, "error with options, use -h for help\n");
            return -1;
        }
    }

    if (optind >= argc) {
        printf("-h for help\n");
        return -1;
    }

    filename = argv[optind];

    chains = chains_read_header(filename);

    printf(FLLD" chains of length %d\n",
           (long long unsigned int) chains->num_chains,
           chains->length);

    chains_destroy(chains);

    return 0;
}
