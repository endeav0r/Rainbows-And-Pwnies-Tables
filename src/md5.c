#include "md5.h"


void md5_hash (unsigned char * sum, unsigned char * data, int data_len)
{
	struct md5_context context;
	int i;
	
	context.A = md5_A;
	context.B = md5_B;
	context.C = md5_C;
	context.D = md5_D;
	
	data_len %= 55;
	
	// turned out faster than memset
	for (i = 0; i < 16; i++)
		context.M[i] ^= context.M[i];
	// faster than memcpy
	for (i = 0; i < data_len / 4; i++)
		context.M[i] = ((unsigned int *) data)[i];

	i <<= 2;
	switch (data_len & 3)
	{
		case 0 :
			((unsigned char *) context.M)[i] = 0x80;
			break;
		case 1 :
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = 0x80;
			break;
		case 2 :
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = 0x80;
			break;
		case 3 :
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = data[i];
			i++;
			((unsigned char *) context.M)[i] = 0x80;
			break;
	}
	
	context.M[14] = data_len << 3; // * 8

	md5_transform(&context);
	
    *((uint32_t *) &(sum[ 0])) = context.A;
    *((uint32_t *) &(sum[ 4])) = context.B;
    *((uint32_t *) &(sum[ 8])) = context.C;
    *((uint32_t *) &(sum[12])) = context.D;
}



void md5_transform (struct md5_context * context)
{

	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	
    a = context->A;
	b = context->B;
	c = context->C;
	d = context->D;

	// round 1
	a = b + (md5_rotl(a + md5_F(b, c, d) + context->M[0]  + md5_T1, 7));
	d = a + (md5_rotl(d + md5_F(a, b, c) + context->M[1]  + md5_T2, 12));
	c = d + (md5_rotl(c + md5_F(d, a, b) + context->M[2]  + md5_T3, 17));
	b = c + (md5_rotl(b + md5_F(c, d, a) + context->M[3]  + md5_T4, 22));

	a = b + (md5_rotl((uint32_t) a + md5_F(b, c, d) + context->M[4]  + md5_T5, 7));
	d = a + (md5_rotl((uint32_t) d + md5_F(a, b, c) + context->M[5]  + md5_T6, 12));
	c = d + (md5_rotl((uint32_t) c + md5_F(d, a, b) + context->M[6]  + md5_T7, 17));
	b = c + (md5_rotl((uint32_t) b + md5_F(c, d, a) + context->M[7]  + md5_T8, 22));

	a = b + (md5_rotl((uint32_t) a + md5_F(b, c, d) + context->M[8]  + md5_T9, 7));
	d = a + (md5_rotl((uint32_t) d + md5_F(a, b, c) + context->M[9]  + md5_T10, 12));
	c = d + (md5_rotl((uint32_t) c + md5_F(d, a, b) + context->M[10] + md5_T11, 17));
	b = c + (md5_rotl((uint32_t) b + md5_F(c, d, a) + context->M[11] + md5_T12, 22));

	a = b + (md5_rotl((uint32_t) a + md5_F(b, c, d) + context->M[12] + md5_T13, 7));
	d = a + (md5_rotl((uint32_t) d + md5_F(a, b, c) + context->M[13] + md5_T14, 12));
	c = d + (md5_rotl((uint32_t) c + md5_F(d, a, b) + context->M[14] + md5_T15, 17));
	b = c + (md5_rotl((uint32_t) b + md5_F(c, d, a) + context->M[15] + md5_T16, 22));

	// round 2
	a = b + (md5_rotl((uint32_t) a + md5_G(b, c, d) + context->M[1]  + md5_T17, 5));
	d = a + (md5_rotl((uint32_t) d + md5_G(a, b, c) + context->M[6]  + md5_T18, 9));
	c = d + (md5_rotl((uint32_t) c + md5_G(d, a, b) + context->M[11] + md5_T19, 14));
	b = c + (md5_rotl((uint32_t) b + md5_G(c, d, a) + context->M[0]  + md5_T20, 20));

	a = b + (md5_rotl((uint32_t) a + md5_G(b, c, d) + context->M[5]  + md5_T21, 5));
	d = a + (md5_rotl((uint32_t) d + md5_G(a, b, c) + context->M[10] + md5_T22, 9));
	c = d + (md5_rotl((uint32_t) c + md5_G(d, a, b) + context->M[15] + md5_T23, 14));
	b = c + (md5_rotl((uint32_t) b + md5_G(c, d, a) + context->M[4]  + md5_T24, 20));

	a = b + (md5_rotl((uint32_t) a + md5_G(b, c, d) + context->M[9]  + md5_T25, 5));
	d = a + (md5_rotl((uint32_t) d + md5_G(a, b, c) + context->M[14] + md5_T26, 9));
	c = d + (md5_rotl((uint32_t) c + md5_G(d, a, b) + context->M[3]  + md5_T27, 14));
	b = c + (md5_rotl((uint32_t) b + md5_G(c, d, a) + context->M[8]  + md5_T28, 20));

	a = b + (md5_rotl((uint32_t) a + md5_G(b, c, d) + context->M[13] + md5_T29, 5));
	d = a + (md5_rotl((uint32_t) d + md5_G(a, b, c) + context->M[2]  + md5_T30, 9));
	c = d + (md5_rotl((uint32_t) c + md5_G(d, a, b) + context->M[7]  + md5_T31, 14));
	b = c + (md5_rotl((uint32_t) b + md5_G(c, d, a) + context->M[12] + md5_T32, 20));

	// round 3
	a = b + (md5_rotl((uint32_t) a + md5_H(b, c, d) + context->M[5]  + md5_T33, 4));
	d = a + (md5_rotl((uint32_t) d + md5_H(a, b, c) + context->M[8]  + md5_T34, 11));
	c = d + (md5_rotl((uint32_t) c + md5_H(d, a, b) + context->M[11] + md5_T35, 16));
	b = c + (md5_rotl((uint32_t) b + md5_H(c, d, a) + context->M[14] + md5_T36, 23));

	a = b + (md5_rotl((uint32_t) a + md5_H(b, c, d) + context->M[1]  + md5_T37, 4));
	d = a + (md5_rotl((uint32_t) d + md5_H(a, b, c) + context->M[4]  + md5_T38, 11));
	c = d + (md5_rotl((uint32_t) c + md5_H(d, a, b) + context->M[7]  + md5_T39, 16));
	b = c + (md5_rotl((uint32_t) b + md5_H(c, d, a) + context->M[10] + md5_T40, 23));

	a = b + (md5_rotl((uint32_t) a + md5_H(b, c, d) + context->M[13] + md5_T41, 4));
	d = a + (md5_rotl((uint32_t) d + md5_H(a, b, c) + context->M[0]  + md5_T42, 11));
	c = d + (md5_rotl((uint32_t) c + md5_H(d, a, b) + context->M[3]  + md5_T43, 16));
	b = c + (md5_rotl((uint32_t) b + md5_H(c, d, a) + context->M[6]  + md5_T44, 23));

	a = b + (md5_rotl((uint32_t) a + md5_H(b, c, d) + context->M[9]  + md5_T45, 4));
	d = a + (md5_rotl((uint32_t) d + md5_H(a, b, c) + context->M[12] + md5_T46, 11));
	c = d + (md5_rotl((uint32_t) c + md5_H(d, a, b) + context->M[15] + md5_T47, 16));
	b = c + (md5_rotl((uint32_t) b + md5_H(c, d, a) + context->M[2] + md5_T48, 23));

	// round 4
	a = b + (md5_rotl((uint32_t) a + md5_I(b, c, d) + context->M[0]  + md5_T49, 6));
	d = a + (md5_rotl((uint32_t) d + md5_I(a, b, c) + context->M[7]  + md5_T50, 10));
	c = d + (md5_rotl((uint32_t) c + md5_I(d, a, b) + context->M[14] + md5_T51, 15));
	b = c + (md5_rotl((uint32_t) b + md5_I(c, d, a) + context->M[5]  + md5_T52, 21));

	a = b + (md5_rotl((uint32_t) a + md5_I(b, c, d) + context->M[12] + md5_T53, 6));
	d = a + (md5_rotl((uint32_t) d + md5_I(a, b, c) + context->M[3]  + md5_T54, 10));
	c = d + (md5_rotl((uint32_t) c + md5_I(d, a, b) + context->M[10] + md5_T55, 15));
	b = c + (md5_rotl((uint32_t) b + md5_I(c, d, a) + context->M[1]  + md5_T56, 21));

	a = b + (md5_rotl((uint32_t) a + md5_I(b, c, d) + context->M[8]  + md5_T57, 6));
	d = a + (md5_rotl((uint32_t) d + md5_I(a, b, c) + context->M[15] + md5_T58, 10));
	c = d + (md5_rotl((uint32_t) c + md5_I(d, a, b) + context->M[6]  + md5_T59, 15));
	b = c + (md5_rotl((uint32_t) b + md5_I(c, d, a) + context->M[13] + md5_T60, 21));

	a = b + (md5_rotl((uint32_t) a + md5_I(b, c, d) + context->M[4]  + md5_T61, 6));
	d = a + (md5_rotl((uint32_t) d + md5_I(a, b, c) + context->M[11] + md5_T62, 10));
	c = d + (md5_rotl((uint32_t) c + md5_I(d, a, b) + context->M[2]  + md5_T63, 15));
	b = c + (md5_rotl((uint32_t) b + md5_I(c, d, a) + context->M[9]  + md5_T64, 21));

	context->A += a;
	context->B += b;
	context->C += c;
	context->D += d;

}

