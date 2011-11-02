#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_CHAINS 32

void print_help ()
{
    printf("rtgen -<cflpn> -[dht]\n");
    printf("\n");
    printf("required arguments\n");
    printf("  -c  <string>  character set\n");
    printf("  -f  <string>  filename to write chains to\n");
    printf("  -l  <int>     length of chains\n");
    printf("  -p  <int>     length of plaintext\n");
    printf("  -n  <int>     number of chains\n");
    printf("  -t  <int>     hash type\n");
    printf("\n");
    printf("hash types:\n");
    printf("  1   MD4\n");
    printf("  2   MD5\n");
    printf("\n");
    printf("optional arguments\n");
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
    char *   filename = NULL;
    int      plaintext_length = -1;
    uint64_t num_chains = 0;
    int      chain_length = -1;
    int      chain_duplicates = 0;
    int      hash_type = 0;
    int      i;

    while ((c = getopt(argc, argv, "c:d:f:hl:n:p:t:")) != -1) {
        switch (c) {
        case 'c' :
            charset = optarg;
            break;
        case 'd' :
            chain_duplicates = atoi(optarg);
            break;
        case 'f' :
            filename = optarg;
            break;
        case 'p' :
            plaintext_length = atoi(optarg);
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

    if (charset == NULL)
        fprintf(stderr, "must give charset.\n");
    if (filename == NULL)
        fprintf(stderr, "must give an output filename\n");
    if (plaintext_length == -1)
        fprintf(stderr, "must give plaintext length.\n");
    if (num_chains == 0)
        fprintf(stderr, "must give a number of chains\n");
    if (hash_type == 0)
        fprintf(stderr, "must give a hash type\n");
    if (chain_length == -1)
        fprintf(stderr, "must give a chain length\n");
    if (    (charset == NULL)
         || (plaintext_length == -1)
         || (num_chains == 0)
         || (filename == NULL)
         || (hash_type == 0)
         || (chain_length == -1)) {
        fprintf(stderr, "use -h for help\n");
        return -1;
    }

    hash      = hash_create(hash_type);
    plaintext = plaintext_create(charset, plaintext_length);

    printf("creating chains. seeding random chain starts\n");
    chains    = chains_create (num_chains);
    printf("sorting seeded chains and removing duplicates\n");
    chains_perfect(chains);
    printf("%lld chains after initial seeding\n",
           (long long int) chains->num_chains);

    if (chain_duplicates > 0) {
        for (i = 1; i * chain_duplicates <= chain_length; i++) {
            printf("expanding chains from %d to %d\n", chains->length, i * chain_duplicates);
            chains_generate(chains, chain_duplicates * i, hash, plaintext);
            printf("sorting and removing duplicates\n");
            chains_perfect(chains);
        }
    }
    else {
        chains_generate(chains, chain_length, hash, plaintext);
        //chains_perfect(chains);
    }

    printf("final chains sort\n");
    chains_sort(chains);

    printf("final chain count: %lld\n", (unsigned long long int) chains->num_chains);
    printf("writing chains\n");
    chains_write(chains, filename);

    hash_destroy(hash);
    plaintext_destroy(plaintext);
    chains_destroy(chains);

    return 0;
}
