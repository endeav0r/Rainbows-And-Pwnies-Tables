#include "nt.h"
#include "md4.h"

void nt_hash (unsigned char * sum, unsigned char * data, int data_len)
{
    int i;
    unsigned char hash_data[48];

    for (i = 0; i < data_len; i++) {
        // our md4 supports only one transform, so we truncate input to NT
        // in order to comply with that
        if (i > 24)
            break;
        hash_data[i<<1]     = data[i];
        hash_data[(i<<1)+1] = '\0';
    }

    md4_hash(sum, hash_data, (data_len <= 24 ? data_len * 2 : 48));
}


