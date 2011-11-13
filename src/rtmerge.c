#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("rtmerge <dst> <src1> <src2>\n");
}

int main (int argc, char * argv[])
{
    _chains * chains_src1;
    _chains * chains_src2;
    _chains * chains_dst;

    int c;
    char *   filename_dst = NULL;
    char *   filename_src1 = NULL;
    char *   filename_src2 = NULL;

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

    if (optind >= argc - 2) {
        printf("-h for help\n");
        return -1;
    }

    filename_dst = argv[optind];
    filename_src1 = argv[optind + 1];
    filename_src2 = argv[optind + 2];

    if ((chains_src1 = chains_read_header(filename_src1)) == NULL)
        fprintf(stderr, "error reading from %s\n", filename_src1);
    if ((chains_src2 = chains_read_header(filename_src2)) == NULL)
        fprintf(stderr, "error reading from %s\n", filename_src2);

    if (chains_src1->length != chains_src2->length) {
        printf("different source chain lengths\n");
        printf("%d and %d\n", chains_src1->length, chains_src2->length);
        return -1;
    }

    printf("src1 with "FLLD" chains\n", (long long unsigned int) chains_src1->num_chains);
    printf("src2 with "FLLD" chains\n", (long long unsigned int) chains_src2->num_chains);

    // open src1 into dst, and then append src2
    if ((chains_dst = chains_read(filename_src1)) == NULL) {
        printf("error opening %s\n", filename_src1);
        return -1;
    }
    if (chains_read_append(chains_dst, filename_src2)) {
        printf("error appending %s to %s\n", filename_src1, filename_src2);
        return -1;
    }

    printf("total of "FLLD" chains, sorting...\n", (long long unsigned int) chains_dst->num_chains);
    chains_sort_random(chains_dst);

    printf("perfecting...\n");
    chains_perfect(chains_dst);

    printf("dst has "FLLD" chains, loss of "FLLD" chains\n",
           (long long unsigned int) chains_dst->num_chains,
           (long long unsigned int)
            ((chains_src1->num_chains + chains_src2->num_chains) - chains_dst->num_chains));

    printf("final sort\n");
    chains_sort(chains_dst);

    chains_write(chains_dst, filename_dst);
    
    chains_destroy(chains_dst);
    chains_destroy(chains_src1);
    chains_destroy(chains_src2);

    return 0;
}
