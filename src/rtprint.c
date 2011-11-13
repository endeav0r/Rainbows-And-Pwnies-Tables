#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("rtprint <chains>\n");
}

int main (int argc, char * argv[])
{
    _chains * chains;

    int c;
    uint64_t i;
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

    chains = chains_read(filename);

    printf(FLLD " chains of length %d\n",
           (long long unsigned int) chains->num_chains,
           chains->length);

    for (i = 0; i < chains->num_chains; i++) {
        printf(F016LLX" "F016LLX" - "F016LLX" "F016LLX"\n",
               (long long unsigned int) chains->chains[i].start_0,
               (long long unsigned int) chains->chains[i].start_1,
               (long long unsigned int) chains->chains[i].end_0,
               (long long unsigned int) chains->chains[i].end_1);
    }

    chains_destroy(chains);

    return 0;
}
