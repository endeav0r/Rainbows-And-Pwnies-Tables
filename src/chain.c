#include "chain.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
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
        CHAINS_MH_BUF[i] += CHAINS_MH_BUF[r % CHAINS_MH_SIZE] + (uint64_t) clock() + 1;
    }

    return r;
}

_chains * chains_create (uint64_t num_chains)
{
    _chains * chains;
    
    chains = (_chains *) malloc(sizeof(_chains));
    chains->num_chains = num_chains;
    chains->length = 0;

    chains->chains = (_chain *) malloc(sizeof(_chain) * num_chains);

    return chains;
}


void chains_seed (_chains * chains)
{
    uint64_t chain_i;

    chains_mini_havege_init();

    for (chain_i = 0; chain_i < chains->num_chains; chain_i++) {
        chains->chains[chain_i].start_0 = chains_mini_havege();
        chains->chains[chain_i].start_1 = chains_mini_havege();
        chains->chains[chain_i].end_0   = chains->chains[chain_i].start_0;
        chains->chains[chain_i].end_1   = chains->chains[chain_i].start_1;
        if ((chain_i & 0x7ffff) == 0)
            printf("seeded %lld of %lld chains\n",
                   (unsigned long long) chain_i,
                   (unsigned long long) chains->num_chains);
    }
}


void chains_destroy (_chains * chains)
{
    if (chains->chains != NULL)
        free(chains->chains);
    free(chains);
}


int chains_generate (_chains * chains, int length, _hash * hash, _plaintext * plaintext)
{
    int num_threads;
    int i;
    uint64_t chunk;
    uint64_t notify;
    uint64_t notify_count;
    int * threads_running;
    pthread_t * threads;
    pthread_attr_t thread_attr;
    _chains_thread_generate * ctgs;
    struct timespec ts, ts_rem;

    if (length <= chains->length)
        return -1;

    ts.tv_sec = 0;
    ts.tv_nsec = 10000;

    // init global thread data
    num_threads = get_nprocs();

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    ctgs    = (_chains_thread_generate *) malloc(sizeof(_chains_thread_generate) * num_threads);
    threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    threads_running = (int *) malloc(sizeof(int) * num_threads);

    // init data for each thread
    for (i = 0; i < num_threads; i++) {
        ctgs[i].chains         = chains;
        ctgs[i].length         = length;
        ctgs[i].hash           = hash_copy(hash);
        ctgs[i].plaintext      = plaintext_copy(plaintext);
        ctgs[i].thread_running = &(threads_running[i]);
        threads_running[i]      = 0;
    }

    notify = 0;
    notify_count = 0;
    for (chunk = 0; chunk < chains->num_chains; chunk += CHAINS_THREAD_CHUNK) {
        i = 0;
        while (i < num_threads) {
            if (threads_running[i] == 0) {
                threads_running[i] = 1;
                ctgs[i].index_start = chunk;
                ctgs[i].index_end   = ((chunk + CHAINS_THREAD_CHUNK) < chains->num_chains)
                                      ? chunk + CHAINS_THREAD_CHUNK : chains->num_chains;
                pthread_create(&(threads[i]), &thread_attr, chains_thread_generate, &(ctgs[i]));
                notify += CHAINS_THREAD_CHUNK;
                if (notify > CHAINS_GENERATE_NOTIFY) {
                    printf("generating chain %lld of %lld\n",
                           (long long unsigned int) (notify * ++notify_count),
                           (long long unsigned int) chains->num_chains);
                    notify = 0;
                }
                break;
            }
            i++;
            if (i == num_threads) {
                nanosleep(&ts, &ts_rem);
                i = 0;
            }
        }
    }

    // wait for all threads to finish
    for (i = 0; i < num_threads; i++) {
        // threads are no longer joinable, so wait for them to die
        while (threads_running[i])
            nanosleep(&ts, &ts_rem);
        hash_destroy(ctgs[i].hash);
        plaintext_destroy(ctgs[i].plaintext);
    }

    // clean up
    free(threads);
    free(threads_running);
    free(ctgs);

    chains->length = length;

    return 0;
}


void * chains_thread_generate (void * ctg_thread_arg)
{
    _chains_thread_generate * ctg = (_chains_thread_generate *) ctg_thread_arg;
    uint64_t i;
    
    for (i = ctg->index_start; i < ctg->index_end; i++) {
        chain_generate(&(ctg->chains->chains[i]),
                       ctg->chains->length,
                       ctg->length,
                       ctg->hash,
                       ctg->plaintext);
    }
    
    *(ctg->thread_running) = 0;
    pthread_exit(NULL);
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
    uint64_t needle_index_0, needle_index_1;

    _chain * search_chains;

    search_chains = malloc(sizeof(_chain) * chains->length);

    hash_from_string(hash, hash_string);
    needle_index_0 = hash_index_0(hash);
    needle_index_1 = hash_index_1(hash);

    printf("generating possible endings\n");
    // generate all the possible endings
    for (depth = 0; depth < chains->length; depth++) {
        search_chains[depth].end_0 = needle_index_0 + depth;
        search_chains[depth].end_1 = needle_index_1;
        chain_generate(&(search_chains[depth]), depth + 1, chains->length, hash, plaintext);
    }

    printf("sorting possibles\n");
    // sort those chains
    chain_sort(search_chains, 0, chains->length - 1);

    // go through our search chains, checking for identical endings

    printf("searching for endings\n");
    chain_i = 0;
    for (depth = 0; depth < chains->length; depth++) {
        while (chain_cmp(&(chains->chains[chain_i]), &(search_chains[depth])) < 0) {
            chain_i++;
            if (chain_i >= chains->num_chains)
                break;
        }
        
        if (chain_i >= chains->num_chains)
            break;
        
        if (chain_cmp(&(chains->chains[chain_i]), &(search_chains[depth])) == 0) {
            false_finds++;

            text = chain_search(&(chains->chains[chain_i]), chains->length, hash, plaintext,
                                needle_index_0, needle_index_1);
            if (text != NULL) {
                printf("\n");
                free(search_chains);
                return text;
            }
            printf(".");
        }
    }

    printf("false finds: %d\n", false_finds);

    free(search_chains);
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
        if (chain_cmp(&(chains->chains[head]), &(chains->chains[tail])) == 0) {
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
        fwrite(&(chains->chains[i].start_0), sizeof(uint64_t), 1, fh);
        fwrite(&(chains->chains[i].start_1), sizeof(uint64_t), 1, fh);
        fwrite(&(chains->chains[i].end_0), sizeof(uint64_t), 1, fh);
        fwrite(&(chains->chains[i].end_1), sizeof(uint64_t), 1, fh);
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
        fread(&(chains->chains[i].start_0), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].start_1), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end_0), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end_1), sizeof(uint64_t), 1, fh);
    }

    fclose(fh);

    return chains;
}


_chains * chains_read_header (char * filename)
{
    _chains * chains;
    FILE * fh;

    fh = fopen(filename, "rb");
    if (fh == NULL)
        return NULL;

    chains = (_chains *) malloc(sizeof(_chains));

    fread(&(chains->num_chains), sizeof(uint64_t), 1, fh);
    fread(&(chains->length), sizeof(int), 1, fh);

    fclose(fh);
    
    chains->chains = NULL;

    return chains;
}


int chains_read_append (_chains * chains, char * filename)
{
    _chains chains_tmp;
    FILE * fh;
    uint64_t i;

    fh = fopen(filename, "rb");
    if (fh == NULL)
        return -1;

    fseek(fh, 0, SEEK_SET);

    fread(&(chains_tmp.num_chains), sizeof(uint64_t), 1, fh);
    fread(&(chains_tmp.length), sizeof(int), 1, fh);

    if (chains_tmp.length != chains->length)
        return -1;
    
    chains_tmp.chains = (_chain *) realloc(chains->chains, sizeof(_chain)
                                           * (chains->num_chains + chains_tmp.num_chains));
    if (chains_tmp.chains == NULL) {
        fprintf(stderr, "chains_read_append realloc fail\n");
        return -1;
    }
    printf("realloc %p %p\n", chains->chains, chains_tmp.chains);
    chains->chains = chains_tmp.chains;

    printf("appending %lld chains to %lld chains\n",
           (long long unsigned int) chains_tmp.num_chains,
           (long long unsigned int) chains->num_chains);

    for (i = chains->num_chains; i < chains->num_chains + chains_tmp.num_chains; i++) {
        fread(&(chains->chains[i].start_0), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].start_1), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end_0), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end_1), sizeof(uint64_t), 1, fh);
    }
    
    fclose(fh);

    chains->num_chains += chains_tmp.num_chains;

    return 0;
}


int chain_generate (_chain * chain, int start_length, int length, _hash * hash, _plaintext * plaintext)
{
    int length_i;
    uint64_t index_0, index_1;

    index_0 = chain->end_0;
    index_1 = chain->end_1;
    for (length_i = start_length; length_i < length; length_i++) {
        // call plaintext generation method directly
        hash_hash(hash, 
                  (unsigned char *) plaintext->plaintext_gen(plaintext->p.p, index_0, index_1),
                  plaintext->plaintext_length);
        index_0  = hash_index_0(hash);
        index_1  = hash_index_1(hash);
        index_0 += length_i;
    }
    chain->end_0 = index_0;
    chain->end_1 = index_1;
    
    return 0;
}


char * chain_search (_chain * chain, int length, _hash * hash, _plaintext * plaintext,
                     uint64_t needle_0, uint64_t needle_1)
{
    int length_i;
    uint64_t index_0, index_1;

    index_0 = chain->start_0;
    index_1 = chain->start_1;
    for (length_i = 0; length_i < length; length_i ++) {
        hash_hash(hash, 
                  (unsigned char *) plaintext_gen(plaintext, index_0, index_1),
                  plaintext->plaintext_length);
        if (    (needle_0 == hash_index_0(hash))
             && (needle_1 == hash_index_1(hash)))
            return plaintext_gen(plaintext, index_0, index_1);
        index_0  = hash_index_0(hash);
        index_1  = hash_index_1(hash);
        index_0 += length_i;
    }
    return NULL;
}


int  chain_cmp  (_chain * a, _chain * b)
{
    if (a->end_0 < b->end_0)
        return -1;
    if (a->end_0 == b->end_0) {
        if (a->end_1 < b->end_1)
            return -1;
        if (a->end_1 == b->end_1)
            return 0;
    }
    return 1;
}

inline void chain_swap (_chain * a, _chain * b)
{
    if (a == b)
        return;
    a->start_0 ^= b->start_0;
    b->start_0 ^= a->start_0;
    a->start_0 ^= b->start_0;
    
    a->start_1 ^= b->start_1;
    b->start_1 ^= a->start_1;
    a->start_1 ^= b->start_1;
    
    a->end_0   ^= b->end_0;
    b->end_0   ^= a->end_0;
    a->end_0   ^= b->end_0;
    
    a->end_1   ^= b->end_1;
    b->end_1   ^= a->end_1;
    a->end_1   ^= b->end_1;
}

uint64_t chain_partition (_chain * chain, uint64_t left, uint64_t right, uint64_t pivot_index)
{
    uint64_t store_index;
    uint64_t i;

    chain_swap(&(chain[right]), &(chain[pivot_index]));
    store_index = left;

    for (i = left; i <= right - 1; i++) {
        if (chain_cmp(&(chain[i]), &(chain[right])) < 0) {
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
    
    if (left >= right)
        return;

    pivot_index = (right + left) / 2;

    pivot_index = chain_partition(chain, left, right, pivot_index);
    if (pivot_index != 0)
        chain_sort(chain, left, pivot_index - 1);
    chain_sort(chain, pivot_index + 1, right);
}
