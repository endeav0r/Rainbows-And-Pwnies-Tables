#include "chain.h"

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define CHAINS_MH_SIZE 16 

uint64_t CHAINS_MH_BUF[CHAINS_MH_SIZE];

void chains_mini_havege_init ()
{
    uint64_t r = clock();
    int i;

    for (i = 0; i < CHAINS_MH_SIZE * 10240; i++) {
        r += (uint64_t) clock();
        r += r / 13;
        r ^= (r + (uint64_t) clock()) << 32;
        if (r & 1)
            r += r % 17;
        if (r & 2)
            r += 3;
        CHAINS_MH_BUF[i % CHAINS_MH_SIZE] += r ^ (uint64_t) clock();
        r += CHAINS_MH_BUF[r % CHAINS_MH_SIZE];
    }
}

uint64_t chains_mini_havege ()
{
    int i;
    uint64_t r = 0;

    for (i = 0; i < CHAINS_MH_SIZE; i++) {
        r += CHAINS_MH_BUF[i];
        CHAINS_MH_BUF[i] += CHAINS_MH_BUF[r % CHAINS_MH_SIZE] + (uint64_t) clock();
    }

    return r;
}

_chains * chains_create (uint64_t num_chains)
{
    uint64_t chain_i;
    _chains * chains;
    
    chains = (_chains *) malloc(sizeof(_chains));
    chains->num_chains = num_chains;
    chains->length = 0;

    chains->chains = (_chain *) malloc(sizeof(_chain) * num_chains);

    chains_mini_havege_init();

    for (chain_i = 0; chain_i < num_chains; chain_i++) {
        chains->chains[chain_i].start = chains_mini_havege();
        chains->chains[chain_i].end   = chains->chains[chain_i].start;
        if (chain_i % 0x80000 == 0)
            printf("seeded %lld of %lld chains\n",
                   (unsigned long long) chain_i,
                   (unsigned long long) num_chains);
    }

    return chains;
}


void chains_destroy (_chains * chains)
{
    free(chains->chains);
    free(chains);
}


int chains_generate (_chains * chains, int length, _hash * hash, _plaintext * plaintext)
{
    uint64_t chain_i;
    int i;
    _hash ** thread_hash;
    _plaintext ** thread_plaintext;

    if (length <= chains->length)
        return -1;

    printf("chains_generate num threads: %d\n", omp_get_max_threads());

    thread_hash = (_hash **) malloc(sizeof(_hash *) * omp_get_max_threads());
    thread_plaintext = (_plaintext **) malloc(sizeof(_plaintext *) * omp_get_max_threads());
    for (i = 0; i < omp_get_max_threads(); i++) {
        thread_hash[i] = hash_copy(hash);
        thread_plaintext[i] = plaintext_copy(plaintext);
    }

    #pragma omp parallel for schedule(dynamic, 1024)
    for (chain_i = 0; chain_i < chains->num_chains; chain_i++) {
        chain_generate(&(chains->chains[chain_i]),
                       chains->length,
                       length,
                       thread_hash[omp_get_thread_num()],
                       thread_plaintext[omp_get_thread_num()]);
        if (((chain_i + 1) % 16384) == 0) {
            printf("chain %lld of %lld done\n",
                   (unsigned long long) chain_i + 1,
                   (unsigned long long) chains->num_chains);
        }
    }

    for (i = 0; i < omp_get_max_threads(); i++) {
        hash_destroy(thread_hash[i]);
        plaintext_destroy(thread_plaintext[i]);
    }
    free(thread_hash);
    free(thread_plaintext);

    chains->length = length;

    return 0;
}


void chains_sort (_chains * chains)
{
    chain_sort(chains->chains, 0, chains->num_chains - 1);
}


char * chains_search (_chains * chains, _hash * hash, _plaintext * plaintext, char * hash_string)
{
    int depth;
    char * text;
    int false_finds = 0;
    int chain_i;
    uint64_t needle_index;

    _chain * search_chains;

    search_chains = malloc(sizeof(_chain) * chains->length);

    hash_from_string(hash, hash_string);
    needle_index = hash_index(hash);

    printf("generating possible endings\n");
    // generate all the possible endings
    for (depth = 0; depth < chains->length; depth++) {
        search_chains[depth].end = needle_index + depth;
        chain_generate(&(search_chains[depth]), depth + 1, chains->length, hash, plaintext);
    }

    printf("sorting possibles\n");
    // sort those chains
    chain_sort(search_chains, 0, chains->length - 1);

    // go through our search chains, checking for identical endings

    printf("searching for endings\n");
    chain_i = 0;
    for (depth = 0; depth < chains->length; depth++) {
        while (chains->chains[chain_i].end < search_chains[depth].end) {
            chain_i++;
            if (chain_i >= chains->num_chains)
                break;
        }
        if (chain_i >= chains->num_chains)
            break;
        // if ends match
        if (chains->chains[chain_i].end == search_chains[depth].end) {
            false_finds++;

            text = chain_search(&(chains->chains[chain_i]), chains->length, hash, plaintext, needle_index);
            if (text != NULL) {
                printf("\n");
                return text;
            }
            printf(".");
        }
    }

    printf("false finds: %d\n", false_finds);

    return NULL;
}


void chains_perfect (_chains * chains)
{
    uint64_t head, tail, swap;

    chains_sort(chains);

    if (chains->num_chains <= 1)
        return;

//  0    1    2    3    4    ...  end
// |    |    |    |    |    |    |    |
//       head tail      swap (duplicates)

    tail = chains->num_chains - 1;
    head = tail - 1;
    swap = tail;

    while (head < chains->num_chains) {
        if (chains->chains[head].end == chains->chains[tail].end) {
            chain_swap(&(chains->chains[tail]), &(chains->chains[swap]));
            swap--;
        }
        head--;
        tail--;
    }

    chains->num_chains = swap + 1;
}


int chains_write (_chains * chains, char * filename)
{
    uint64_t i;
    FILE * fh;

    fh = fopen(filename, "wb");
    if (fh == NULL)
        return -1;

    fwrite(&(chains->num_chains), sizeof(uint64_t), 1, fh);
    fwrite(&(chains->length), sizeof(int), 1, fh);

    for (i = 0; i < chains->num_chains; i++) {
        fwrite(&(chains->chains[i].start), sizeof(uint64_t), 1, fh);
        fwrite(&(chains->chains[i].end), sizeof(uint64_t), 1, fh);
    }

    fclose(fh);

    return 0;
}


_chains * chains_read (char * filename)
{
    _chains * chains;
    FILE * fh;
    uint64_t i;

    fh = fopen(filename, "rb");
    if (fh == NULL)
        return NULL;

    chains = (_chains *) malloc(sizeof(_chains));

    fread(&(chains->num_chains), sizeof(uint64_t), 1, fh);
    fread(&(chains->length), sizeof(int), 1, fh);

    chains->chains = (_chain *) malloc(sizeof(_chain) * chains->num_chains);
    for (i = 0; i < chains->num_chains; i++) {
        fread(&(chains->chains[i].start), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end), sizeof(uint64_t), 1, fh);
    }

    fclose(fh);

    return chains;
}


int chain_generate (_chain * chain, int start_length, int length, _hash * hash, _plaintext * plaintext)
{
    int length_i;
    uint64_t index;

    index = chain->end;
    for (length_i = start_length; length_i < length; length_i++) {
        hash_hash(hash, 
                  (unsigned char *) plaintext_gen(plaintext, index),
                  plaintext->plaintext_length);
        index =  hash_index(hash);
        index += length_i;
    }
    chain->end = index;
    
    return 0;
}


char * chain_search (_chain * chain, int length, _hash * hash, _plaintext * plaintext, uint64_t needle)
{
    int length_i;
    uint64_t index;

    index = chain->start;
    for (length_i = 0; length_i < length; length_i ++) {
        hash_hash(hash, 
                  (unsigned char *) plaintext_gen(plaintext, index),
                  plaintext->plaintext_length);
        if (needle == hash_index(hash))
            return plaintext_gen(plaintext, index);
        index = hash_index(hash);
        index += length_i;
    }
    return NULL;
}


inline void chain_swap (_chain * a, _chain * b)
{
    if (a == b)
        return;
    a->start ^= b->start;
    b->start ^= a->start;
    a->start ^= b->start;
    a->end   ^= b->end;
    b->end   ^= a->end;
    a->end   ^= b->end;
}

uint64_t chain_partition (_chain * chain, uint64_t left, uint64_t right, uint64_t pivot_index)
{
    uint64_t pivot_value;
    uint64_t store_index;
    uint64_t i;

    pivot_value = chain[pivot_index].end;

    chain_swap(&(chain[right]), &(chain[pivot_index]));
    store_index = left;

    for (i = left; i < right; i++) {
        if (chain[i].end < pivot_value) {
            chain_swap(&(chain[i]), &(chain[store_index]));
            store_index++;
        }
    }
    chain_swap(&(chain[store_index]), &(chain[right]));

    return store_index;
}

void chain_sort (_chain * chain, uint64_t left, uint64_t right)
{
    uint64_t pivot_index;
    
    if ((left >= right) || ((int64_t) right < 0))
        return;

    pivot_index = (right + left) / 2;

    pivot_index = chain_partition(chain, left, right, pivot_index);

    chain_sort(chain, left, pivot_index - 1);
    chain_sort(chain, pivot_index + 1, right);
}
