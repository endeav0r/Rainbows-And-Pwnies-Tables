#include "chain.h"
#include "markov.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_help ()
{
    printf("markovgen <mask> <iterations>\n");
}

int main (int argc, char * argv[])
{
    _plaintext * plaintext;
    int iterations;
    
    if (argc != 3) {
        print_help();
        return -1;
    }
    
    plaintext = plaintext_create(PLAINTEXT_TYPE_MASK, argv[1], -1);
    iterations = atoi(argv[2]);
    chains_mini_havege_init();
    
    while (iterations--) {
        printf("%s\n", plaintext_gen(plaintext, chains_mini_havege()));
    }

    plaintext_destroy(plaintext);

    return 0;
}
