#include "chain.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#ifdef USE_THREADS
    #ifdef MINGW
        #include <windows.h>
    #else
        #include <sys/sysinfo.h>
    #endif
#endif

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
        chains->chains[chain_i].start = chains_mini_havege();
        chains->chains[chain_i].end   = chains->chains[chain_i].start;
        if ((chain_i & 0x7ffff) == 0)
            printf("seeded "FLLD" of "FLLD" chains\n",
                   (long long unsigned int) chain_i,
                   (long long unsigned int) chains->num_chains);
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
    uint64_t chunk;
    uint64_t notify;
    uint64_t notify_count;

    #ifdef USE_THREADS
        int i;
        int num_threads;
        int * threads_running;
        pthread_t * threads;
        pthread_attr_t thread_attr;
        _chains_thread_generate * ctgs;
        #ifdef MINGW
            SYSTEM_INFO sysinfo;
        #else
            struct timespec ts, ts_rem;
        #endif
    #endif

    if (length <= chains->length)
        return -1;

    #ifdef USE_THREADS

        // init global thread data
        #ifdef MINGW
            GetSystemInfo(&sysinfo);
            num_threads = sysinfo.dwNumberOfProcessors;
        #else
            num_threads = get_nprocs();
            ts.tv_sec = 0;
            ts.tv_nsec = 10000;
        #endif

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
    #endif

    notify = 0;
    notify_count = 0;

    #ifdef USE_THREADS
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
                        printf("generating chain "FLLD" of "FLLD"\n",
                            (long long unsigned int) (notify * ++notify_count),
                            (long long unsigned int) chains->num_chains);
                        notify = 0;
                    }
                    break;
                }
                i++;
                if (i == num_threads) {
                    #ifdef MINGW
                        Sleep(10);
                    #else
                        nanosleep(&ts, &ts_rem);
                    #endif
                    i = 0;
                }
            }
        }
    
        // wait for all threads to finish
        for (i = 0; i < num_threads; i++) {
            // threads are no longer joinable, so wait for them to die
            while (threads_running[i])
                #ifdef MINGW
                    Sleep(10);
                #else
                    nanosleep(&ts, &ts_rem);
                #endif
            hash_destroy(ctgs[i].hash);
            plaintext_destroy(ctgs[i].plaintext);
        }

        // clean up
        free(threads);
        free(threads_running);
        free(ctgs);
    #else
        for (chunk = 0; chunk < chains->num_chains; chunk++) {
            chain_generate(&(chains->chains[chunk]), chains->length, length, hash, plaintext);
            notify++;
            if (notify > CHAINS_GENERATE_NOTIFY) {
                printf("generating chain "FLLD" of "FLLD"\n",
                    (long long unsigned int) (notify * ++notify_count),
                    (long long unsigned int) chains->num_chains);
                notify = 0;
            }
        }
    #endif

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
    #ifdef USE_THREDS
        pthread_exit(NULL);
    #else
        return NULL;
    #endif

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
                                needle_index);
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


void chains_sort (_chains * chains)
{
    chain_sort(chains->chains, 0, chains->num_chains - 1);
}


void chains_sort_random (_chains * chains)
{
    uint64_t i;

    // this is memory intensive, but we make up for it with faster quicksort later
    for (i = 0; i < chains->num_chains; i++)
        chain_swap(&(chains->chains[i]),
                   &(chains->chains[chains->chains[i].start % chains->num_chains]));

    chain_sort (chains->chains, 0, chains->num_chains - 1);
}


// returns -1 if chain verification failed, 0 on success
int chains_verify (_chains * chains, uint64_t chains_to_verify, _hash * hash, _plaintext * plaintext)
{
    uint64_t i;
    uint64_t index;
    _chain chain;

    chain.end = 0;

    chains_mini_havege_init();

    for (i = 0; i < chains_to_verify; i++) {
        index = chains_mini_havege() % chains->num_chains;
        chain.end = chains->chains[index].start;
        chain_generate(&chain, 0, chains->length, hash, plaintext);
        if (chain_cmp(&(chains->chains[index]), &chain)) {
            printf("verification round "FLLD"\n", (unsigned long long int) i);
            printf("index "FLLD"\n", (unsigned long long int) index);
            printf("start    "F016LLX"\n",
                   (unsigned long long int) chains->chains[index].start);
            printf("expected "F016LLX"\n",
                   (unsigned long long int) chains->chains[index].end);
            printf("received "F016LLX"\n",
                   (unsigned long long int) chain.end);
            return -1;
        }
    }

    return 0;
}



int chains_write (_chains * chains, char * filename)
{
    uint64_t i;
    FILE * fh;
    char tmp_filename[512];
    int error;

    snprintf(tmp_filename, 512, "%s.tmp", filename);

    fh = fopen(tmp_filename, "wb");
    if (fh == NULL)
        return -1;

    fwrite(&(chains->num_chains), sizeof(uint64_t), 1, fh);
    fwrite(&(chains->length), sizeof(int), 1, fh);

    for (i = 0; i < chains->num_chains; i++) {
        fwrite(&(chains->chains[i].start), sizeof(uint64_t), 1, fh);
        fwrite(&(chains->chains[i].end), sizeof(uint64_t), 1, fh);
    }

    fclose(fh);
    
    unlink(filename);

    error = rename(tmp_filename, filename);
    if (error) {
        fprintf(stderr, "error %d moving file %s to %s\n",
                error, tmp_filename, filename);
        return -1;
    }

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

    printf("appending "FLLD" chains to "FLLD" chains\n",
           (long long unsigned int) chains_tmp.num_chains,
           (long long unsigned int) chains->num_chains);

    for (i = chains->num_chains; i < chains->num_chains + chains_tmp.num_chains; i++) {
        fread(&(chains->chains[i].start), sizeof(uint64_t), 1, fh);
        fread(&(chains->chains[i].end), sizeof(uint64_t), 1, fh);
    }
    
    fclose(fh);

    chains->num_chains += chains_tmp.num_chains;

    return 0;
}


int chain_generate (_chain * chain, int start_length, int length, _hash * hash, _plaintext * plaintext)
{
    int length_i;
    uint64_t index;

    index = chain->end;
    for (length_i = start_length; length_i < length; length_i++) {
        // call plaintext generation method directly
        hash_hash(hash, 
                  (unsigned char *) plaintext->plaintext_gen(plaintext->p.p, index),
                  plaintext->plaintext_length);
        index  = hash_index(hash);
        index += length_i;
    }
    chain->end = index;
    
    return 0;
}


char * chain_search (_chain * chain, int length, _hash * hash, _plaintext * plaintext,
                     uint64_t needle)
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
        index  = hash_index(hash);
        index += length_i;
    }
    return NULL;
}


int  chain_cmp  (_chain * a, _chain * b)
{
    if (a->end > b->end)
        return 1;
    if (a->end < b->end)
        return -1;
    return 0;
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
