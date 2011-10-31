#include "plaintext.h"
#include "chain.h"
#include "hash.h"

#include "md4.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("rtgen -<cfp> -[h] <hash>\n");
    printf("\n");
    printf("required arguments\n");
    printf("  -c  <string>  character set\n");
    printf("  -f  <string>  filename to read chains from\n");
    printf("  -p  <int>     length of plaintext\n");
    printf("\n");
    printf("optional arguments\n");
    printf("  -h            this help message\n");
}

int main (int argc, char * argv[])
{
    _hash * hash;
    _plaintext * plaintext;
    _chains * chains;

    int c;
    char * charset = NULL;
    char * filename = NULL;
    int    plaintext_length = -1;
    
    while ((c = getopt(argc, argv, "c:f:p:h")) != -1) {
        switch (c) {
        case 'c' :
            charset = optarg;
            break;
        case 'f' :
            filename = optarg;
            break;
        case 'p' :
            plaintext_length = atoi(optarg);
            break;
        case 'h' :
            print_help();
            return 0;
        case '?' :
            fprintf(stderr, "error with options, use -h for help\n");
            return -1;
        }
    }

    if (charset == NULL)
        fprintf(stderr, "must give charset.\n");
    if (filename == NULL)
        fprintf(stderr, "must give an output filename.\n");
    if (plaintext_length == -1)
        fprintf(stderr, "must give a plaintext length.\n");
    if (optind >= argc)
        fprintf(stderr, "must give hash as argument.\n");
    if (    (charset == NULL)
         || (filename == NULL)
         || (plaintext_length == -1)
         || (optind >= argc)) {
        fprintf(stderr, "use -h for help\n");
        return -1;
    }

    hash      = hash_create(16, md4_hash);
    plaintext = plaintext_create(charset, plaintext_length);

    chains = chains_read(filename);

    printf("%lld chains of length %d\n",
           (long long int) chains->num_chains, chains->length);
    printf("searching chains\n");fflush(stdout);
    printf("%s\n", chains_search(chains, hash, plaintext, argv[optind]));

    hash_destroy(hash);
    plaintext_destroy(plaintext);
    chains_destroy(chains);

    return 0;
}
