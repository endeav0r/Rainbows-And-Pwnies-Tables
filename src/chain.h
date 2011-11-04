#ifndef chain_HEADER
#define chain_HEADER

#include <stdint.h>
#include <pthread.h>

#include "plaintext.h"
#include "hash.h"

#define CHAINS_THREAD_CHUNK 2048 
#define CHAINS_GENERATE_NOTIFY 0x10000

typedef struct _chain_s {
    uint64_t start;
    uint64_t end;
} _chain;

typedef struct _chains_s {
    uint64_t num_chains;
    int length;
    _chain * chains;
} _chains;

typedef struct _chains_thread_generate_s {
    _chains *    chains;
    uint64_t     index_start;
    uint64_t     index_end;
    int          length;
    _hash *      hash;
    _plaintext * plaintext;
    int *        thread_running;
} _chains_thread_generate;

void     chains_mini_havege_init();
uint64_t chains_mini_havege();

_chains * chains_create  (uint64_t num_chains);
void      chains_destroy (_chains * chains);

int    chains_generate (_chains * chains, int length, _hash * hash, _plaintext * plaintext);
void   chains_sort     (_chains * chains);
char * chains_search   (_chains * chains, _hash * hash, _plaintext * plaintext, char * hash_string);
void   chains_perfect  (_chains * chains);

void * chains_thread_generate (void * ctg_thread_arg);

int       chains_write        (_chains * chains, char * filename);
_chains * chains_read         (char * filename);
_chains * chains_read_header  (char * filename);
int       chains_read_append  (_chains * chains, char * filename);


int         chain_generate (_chain * chain, int start_index, int length, _hash * hash, _plaintext * plaintext);
char *      chain_search   (_chain * chain, int length, _hash * hash, _plaintext * plaintext, uint64_t needle);
inline void chain_swap     (_chain * a, _chain * b);
void        chain_sort     (_chain * chain, uint64_t left, uint64_t right);

#endif
