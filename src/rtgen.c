#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("rtgen -<cflmpnt> -[bdh]\n");
    printf("\n");
    printf("required arguments:\n");
    printf("  -c  <string>  character set\n");
    printf("  -f  <string>  filename to write chains to\n");
    printf("  -k  <string>  use mask for plaintext generation\n");
    printf("  -l  <int>     length of chains\n");
    printf("  -p  <int>     length of plaintext\n");
    printf("  -m  <string>  markov model filename\n");
    printf("  -n  <int>     number of chains\n");
    printf("  -t  <int>     hash type\n");
    printf("\n");
    printf("plaintext types:\n");
    printf("  passing -c will choose bruteforce plaintext generation\n");
    printf("  passing -k will choose password mask generation\n");
    printf("  passing -m will choose markov plaintext generation\n");
    printf("  one and only of these options must be passed\n");
    printf("\n");
    printf("hash types:\n");
    printf("  1   MD4\n");
    printf("  2   MD5\n");
    printf("  3   NT\n");
    printf("\n");
    printf("optional arguments\n");
    printf("  -b            benchmark. won't write chains or do intensive seeds\n");
    printf("  -d  <int>     chain length interval to remove duplicates\n");
    printf("  -h            this help message\n");
}

int main (int argc, char * argv[])
{
    _hash * hash;
    _plaintext * plaintext;
    _chains * chains;

    int c;
    char *   charset = NULL;
    char *   mask = NULL;
    char *   markov = NULL;
    char *   filename = NULL;
    int      plaintext_length = -1;
    uint64_t num_chains = 0;
    int      chain_length = -1;
    int      chain_duplicates = 0;
    int      hash_type = 0;
    int      benchmark = 0;
    int      i;

    while ((c = getopt(argc, argv, "bc:d:f:hk:l:m:n:p:t:")) != -1) {
        switch (c) {
        case 'b' :
            benchmark = 1;
            break;
        case 'c' :
            charset = optarg;
            break;
        case 'd' :
            chain_duplicates = atoi(optarg);
            break;
        case 'f' :
            filename = optarg;
            break;
        case 'k' :
            mask = optarg;
            break;
        case 'p' :
            plaintext_length = atoi(optarg);
            break;
        case 'm' :
            markov = optarg;
            break;
        case 'n' :
            num_chains = strtoull(optarg, NULL, 10);
            break;
        case 'l' :
            chain_length = atoi(optarg);
            break;
        case 'h' :
            print_help();
            return 0;
        case 't' :
            hash_type = atoi(optarg);
            break;
        case '?' :
            fprintf(stderr, "error with options, use -h for help\n");
            return -1;
        }
    }

    if ((charset == NULL) && (markov == NULL) && (mask == NULL))
        fprintf(stderr, "must give charset, mask or markov model filename\n");
    if (filename == NULL)
        fprintf(stderr, "must give an output filename\n");
    if ((plaintext_length == -1) && (mask == NULL))
        fprintf(stderr, "must give plaintext length\n");
    if (num_chains == 0)
        fprintf(stderr, "must give a number of chains\n");
    if (hash_type == 0)
        fprintf(stderr, "must give a hash type\n");
    if (chain_length == -1)
        fprintf(stderr, "must give a chain length\n");
    if (    ((plaintext_length == -1) && (mask == NULL))
         || (num_chains == 0)
         || (filename == NULL)
         || (hash_type == 0)
         || (chain_length == -1)
         || ((charset == NULL) && (markov == NULL) && (mask == NULL))) {
        fprintf(stderr, "use -h for help\n");
        return -1;
    }

    hash = hash_create(hash_type);
    printf("plaintext length: %d\n", plaintext_length);
    if (mask != NULL)
        plaintext = plaintext_create(PLAINTEXT_TYPE_MASK, mask, strlen(mask));
    else if (charset != NULL)
        plaintext = plaintext_create(PLAINTEXT_TYPE_BRUTEFORCE, charset, plaintext_length);
    else 
        plaintext = plaintext_create(PLAINTEXT_TYPE_MARKOV, markov, plaintext_length);

    printf("creating chains\n");
    chains    = chains_create (num_chains);
    if (benchmark == 0) {
        printf("seeding chains\n");
        chains_seed(chains);
        printf("sorting seeded chains and removing duplicates\n");
        chains_sort(chains);
        chains_perfect(chains);
        printf(FLLD" chains after initial seeding\n",
            (long long int) chains->num_chains);
    }
    else
        printf("benchmark, not seeding chains\n");
    printf("chain generation complete\n");

    if (chain_duplicates > 0) {
        for (i = 1; i * chain_duplicates <= chain_length; i++) {
            printf("expanding chains from %d to %d\n", chains->length, i * chain_duplicates);
            fflush(stdout);
            chains_generate(chains, chain_duplicates * i, hash, plaintext);
            printf("sorting and removing duplicates\n");
            fflush(stdout);
            chains_sort(chains);
            chains_perfect(chains);
        }
    }
    else 
        chains_generate(chains, chain_length, hash, plaintext);

    printf("final chain count: "FLLD"\n", (unsigned long long int) chains->num_chains);

    if (benchmark == 0) {
        printf("final chains sort\n");
        chains_sort(chains);

        printf("writing chains\n");
        chains_write(chains, filename);
    }

    hash_destroy(hash);
    plaintext_destroy(plaintext);
    chains_destroy(chains);

    return 0;
}
