#include "chain.h"
#include "markov.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("markovgen <markov_model> <number_plaintexts_to_gen>\n");
}

int main (int argc, char * argv[])
{
    _plaintext * plaintext;

    int plaintexts_to_gen;
    char * model_filename = NULL;
    uint64_t seed_0, seed_1;
    int c;

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

    if (optind >= argc + 1) {
        printf("-h for help\n");
        return -1;
    }

    model_filename = argv[optind];
    plaintexts_to_gen = atoi(argv[optind + 1]);
    
    plaintext = plaintext_create(PLAINTEXT_TYPE_MARKOV, model_filename, 10);
    chains_mini_havege_init();
    while (plaintexts_to_gen-- > 0) {
        seed_0 = chains_mini_havege();
        seed_1 = chains_mini_havege();
        printf("%s\n", plaintext_gen(plaintext, seed_0, seed_1));
    }

    plaintext_destroy(plaintext);

    return 0;
}
