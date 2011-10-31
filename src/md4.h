#ifndef md4_HEADER
#define md4_HEADER

/*
	Only use for md4 hashes < ~50 bytes
	fasssttttttttt
*/

#include <stdint.h>
#include <string.h>

#define md4_A 0x67452301
#define md4_B 0xefcdab89
#define md4_C 0x98badcfe
#define md4_D 0x10325476

#define md4_rotl(x, s) (((x) << s) | ((x) >> (32 - s)))
#define md4_F(x, y, z) ((x & y) | ((~x) & z))
#define md4_G(x, y, z) ((x & y) | (x & z) | (y & z))
#define md4_H(x, y, z) (x ^ y ^ z)
#define md4_l2bendian(x) (((x) << 24) | ((x) >> 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8))

struct md4_context
{
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t M[16];
};

uint64_t md4_sum_to_index (unsigned char * sum);
void     md4_hash         (unsigned char * sum, unsigned char * data, int data_size);

inline void md4_transform (struct md4_context * context);

#endif
