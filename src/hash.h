#ifndef hash_HEADER
#define hash_HEADER

#include <stdint.h>

#define HASH_SUM_MAX_SIZE 64
#define HASH_FUNC void (* hash_func) (unsigned char * sum, unsigned char * data, int data_size)

typedef struct _hash_s {
    int sum_size;
    HASH_FUNC;
    unsigned char sum[HASH_SUM_MAX_SIZE];
} _hash;

_hash *  hash_create  (int sum_size, HASH_FUNC);
void     hash_destroy (_hash * hash);

int      hash_from_string (_hash * hash, char * string);

void     hash_hash  (_hash * hash, unsigned char * data, int data_len);
uint64_t hash_index (_hash * hash);

#endif
