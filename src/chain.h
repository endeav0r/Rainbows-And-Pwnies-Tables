#ifndef chain_HEADER
#define chain_HEADER

#include <stdint.h>

#include "plaintext.h"
#include "hash.h"

typedef struct _chain_s {
    uint64_t start;
    uint64_t end;
} _chain;

typedef struct _chains_s {
    uint64_t num_chains;
    int length;
    _chain * chains;
} _chains;

void     chains_mini_havege_init();
uint64_t chains_mini_havege();

_chains * chains_create  (uint64_t num_chains);
void      chains_destroy (_chains * chains);

int    chains_generate (_chains * chains, int length, _hash * hash, _plaintext * plaintext);
void   chains_sort     (_chains * chains);
char * chains_search   (_chains * chains, _hash * hash, _plaintext * plaintext, char * hash_string);
void   chains_perfect  (_chains * chains);

int       chains_write (_chains * chains, char * filename);
_chains * chains_read  (char * filename);

int         chain_generate (_chain * chain, int start_index, int length, _hash * hash, _plaintext * plaintext);
char *      chain_search   (_chain * chain, int length, _hash * hash, _plaintext * plaintext, uint64_t needle);
inline void chain_swap     (_chain * a, _chain * b);
void        chain_sort     (_chain * chain, uint64_t left, uint64_t right);

#endif
