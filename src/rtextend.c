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
    printf("rtextend -<cfilmpt> -[dh]\n");
    printf("\n");
    printf("required arguments\n");
    printf("  -c  <string>  character set\n");
    printf("  -f  <string>  filename to write chains to\n");
    printf("  -i  <string>  filename to read chains from\n");
    printf("  -k  <string>  use mask for plaintext generation\n");
    printf("  -l  <int>     length to extend chains to\n");
    printf("  -m  <string>  markov model filename\n");
    printf("  -p  <int>     length of plaintext\n");
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
    printf("  -d  <int>     chain length interval to remove duplicates\n");
    printf("  -h            this help message\n");
}

int main (int argc, char * argv[])
{
    _hash * hash;
    _plaintext * plaintext;
    _chains * chains;

    int c;
    char *   charset          = NULL;
    char *   mask             = NULL;
    char *   markov           = NULL;
    char *   filename_in      = NULL;
    char *   filename_out     = NULL;
    int      plaintext_length = -1;
    int      chain_length     = -1;
    int      chain_duplicates = 0;
    int      hash_type        = 0;
    int      i;

    while ((c = getopt(argc, argv, "c:d:f:i:k:hl:m:p:t:")) != -1) {
        switch (c) {
        case 'c' :
            charset = optarg;
            break;
        case 'd' :
            chain_duplicates = atoi(optarg);
            break;
        case 'f' :
            filename_out = optarg;
            break;
        case 'i' :
            filename_in = optarg;
            break;
        case 'k' :
            mask = optarg;
            break;
        case 'm' :
            markov = optarg;
            break;
        case 'p' :
            plaintext_length = atoi(optarg);
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
    if (filename_out == NULL)
        fprintf(stderr, "must give an output filename\n");
    if ((plaintext_length == -1) && (mask == NULL))
        fprintf(stderr, "must give plaintext length.\n");
    if (filename_in == NULL)
        fprintf(stderr, "must give an input filename\n");
    if (hash_type == 0)
        fprintf(stderr, "must give a hash type\n");
    if (chain_length == -1)
        fprintf(stderr, "must give a chain length\n");
    if (    ((plaintext_length == -1) && (mask == NULL))
         || (filename_in == NULL)
         || (filename_out == NULL)
         || (hash_type == 0)
         || (chain_length == -1)
         || ((charset == NULL) && (markov == NULL) && (mask == NULL))) {
        fprintf(stderr, "use -h for help\n");
        return -1;
    }

    hash = hash_create(hash_type);
    if (mask != NULL)
        plaintext = plaintext_create(PLAINTEXT_TYPE_MASK, mask, strlen(mask));
    else if (charset != NULL)
        plaintext = plaintext_create(PLAINTEXT_TYPE_BRUTEFORCE, charset, plaintext_length);
    else 
        plaintext = plaintext_create(PLAINTEXT_TYPE_MARKOV, markov, plaintext_length);

    printf("reading chains\n");
    chains    = chains_read (filename_in);
    printf(FLLD" chains of length %d read\n",
           (unsigned long long int) chains->num_chains,
           chains->length);

    if (chain_duplicates > 0) {
        for (i = chains->length / chain_duplicates + 1; i * chain_duplicates <= chain_length; i++) {
            printf("expanding chains from %d to %d\n", chains->length, i * chain_duplicates);
            chains_generate(chains, chain_duplicates * i, hash, plaintext);
            printf("sorting and removing duplicates\n");
            chains_sort(chains);
            chains_perfect(chains);
            chains_write(chains, filename_out);
        }
    }
    else {
        chains_generate(chains, chain_length, hash, plaintext);
        chains_sort(chains);
        chains_perfect(chains);
    }

    printf("final chains sort\n");
    chains_sort(chains);

    printf("final chain count: "FLLD"\n", (unsigned long long int) chains->num_chains);
    printf("writing chains\n");
    chains_write(chains, filename_out);

    hash_destroy(hash);
    plaintext_destroy(plaintext);
    chains_destroy(chains);

    return 0;
}
