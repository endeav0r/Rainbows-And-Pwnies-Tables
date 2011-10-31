#ifndef md5_HEADER
#define md5_HEADER

#include <stdint.h>
#include <string.h>

#define md5_T1 0xd76aa478
#define md5_T2 0xe8c7b756
#define md5_T3 0x242070db
#define md5_T4 0xc1bdceee
#define md5_T5 0xf57c0faf
#define md5_T6 0x4787c62a
#define md5_T7 0xa8304613
#define md5_T8 0xfd469501
#define md5_T9 0x698098d8
#define md5_T10 0x8b44f7af
#define md5_T11 0xffff5bb1
#define md5_T12 0x895cd7be
#define md5_T13 0x6b901122
#define md5_T14 0xfd987193
#define md5_T15 0xa679438e
#define md5_T16 0x49b40821
#define md5_T17 0xf61e2562
#define md5_T18 0xc040b340
#define md5_T19 0x265e5a51
#define md5_T20 0xe9b6c7aa
#define md5_T21 0xd62f105d
#define md5_T22 0x2441453
#define md5_T23 0xd8a1e681
#define md5_T24 0xe7d3fbc8
#define md5_T25 0x21e1cde6
#define md5_T26 0xc33707d6
#define md5_T27 0xf4d50d87
#define md5_T28 0x455a14ed
#define md5_T29 0xa9e3e905
#define md5_T30 0xfcefa3f8
#define md5_T31 0x676f02d9
#define md5_T32 0x8d2a4c8a
#define md5_T33 0xfffa3942
#define md5_T34 0x8771f681
#define md5_T35 0x6d9d6122
#define md5_T36 0xfde5380c
#define md5_T37 0xa4beea44
#define md5_T38 0x4bdecfa9
#define md5_T39 0xf6bb4b60
#define md5_T40 0xbebfbc70
#define md5_T41 0x289b7ec6
#define md5_T42 0xeaa127fa
#define md5_T43 0xd4ef3085
#define md5_T44 0x4881d05
#define md5_T45 0xd9d4d039
#define md5_T46 0xe6db99e5
#define md5_T47 0x1fa27cf8
#define md5_T48 0xc4ac5665
#define md5_T49 0xf4292244
#define md5_T50 0x432aff97
#define md5_T51 0xab9423a7
#define md5_T52 0xfc93a039
#define md5_T53 0x655b59c3
#define md5_T54 0x8f0ccc92
#define md5_T55 0xffeff47d
#define md5_T56 0x85845dd1
#define md5_T57 0x6fa87e4f
#define md5_T58 0xfe2ce6e0
#define md5_T59 0xa3014314
#define md5_T60 0x4e0811a1
#define md5_T61 0xf7537e82
#define md5_T62 0xbd3af235
#define md5_T63 0x2ad7d2bb
#define md5_T64 0xeb86d391

#define md5_A 0x67452301
#define md5_B 0xefcdab89
#define md5_C 0x98badcfe
#define md5_D 0x10325476

#define md5_rotl(x, s) (((x) << s) | ((x) >> (32 - s)))
#define md5_F(x, y, z) ((x & y) | ((~x) & z))
#define md5_G(x, y, z) ((x & z) | (y & ~z))
#define md5_H(x, y, z) (x ^ y ^ z)
#define md5_I(x, y, z) (y ^ (x | ~z))
#define md5_l2bendian(x) (((x) << 24) | ((x) >> 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8))


struct md5_context
{
        uint32_t A; // digest output
        uint32_t B; // digest output
        uint32_t C; // digest output
        uint32_t D; // digest output
        uint32_t M[16]; // where the original data is stored
};


uint64_t md5_sum_to_index (unsigned char * sum);
void     md5_hash         (unsigned char * sum, unsigned char * data, int data_size);

inline void md5_transform (struct md5_context * context);

#endif
