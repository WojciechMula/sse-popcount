/*
	Bit population count, $Revision$

	This program includes following functions:
	* lookup  --- lookup based 
	* ssse3-1 --- SSSE3 using PSHUFB and PSADBW
	* ssse3-2 --- improved SSSE3 procedure - PSADBW called fewer times
	* sse2-1  --- bit-parallel counting and PSADBW
	* sse2-2  --- bit-parallel counting - PSADBW called fewer times (the same
	              optimization as in ssse3-2)
	* ssse3-unrl --- ssse3-2 with inner loop unrolled 4 times
	* sse2-unrl --- ssse2-2 with inner loop unrolled 4 times
	
	compilation:
	$ gcc -O3 -Wall -pedantic -std=c99 ssse3_popcount.c
	
	Author: Wojciech Mu³a
	e-mail: wojciech_mula@poczta.onet.pl
	www:    http://0x80.pl/
	
	License: BSD
	
	initial release 24-05-2008, last update $Date$
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#ifdef ALIGN_DATA
#	define __aligned__ __attribute__((aligned(16)))
#else
#	define __aligned__
#endif

#define MAX_CHUNKS 2048

uint8_t buffer[16*MAX_CHUNKS] __aligned__;

// lookup for SSE
uint8_t POPCOUNT_4bit[16] __aligned__ = {
	/* 0 */ 0,
	/* 1 */ 1,
	/* 2 */ 1,
	/* 3 */ 2,
	/* 4 */ 1,
	/* 5 */ 2,
	/* 6 */ 2,
	/* 7 */ 3,
	/* 8 */ 1,
	/* 9 */ 2,
	/* a */ 2,
	/* b */ 3,
	/* c */ 2,
	/* d */ 3,
	/* e */ 3,
	/* f */ 4
};


// ---- lookup[256] -------------------------------------------------------

// uint32_t instead of uint8_t, to avoid zero-extend
uint32_t POPCOUNT_8bit[256] __aligned__ = {
	/* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
	/* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
	/* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
	/* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4,
	/* 10 */ 1, /* 11 */ 2, /* 12 */ 2, /* 13 */ 3,
	/* 14 */ 2, /* 15 */ 3, /* 16 */ 3, /* 17 */ 4,
	/* 18 */ 2, /* 19 */ 3, /* 1a */ 3, /* 1b */ 4,
	/* 1c */ 3, /* 1d */ 4, /* 1e */ 4, /* 1f */ 5,
	/* 20 */ 1, /* 21 */ 2, /* 22 */ 2, /* 23 */ 3,
	/* 24 */ 2, /* 25 */ 3, /* 26 */ 3, /* 27 */ 4,
	/* 28 */ 2, /* 29 */ 3, /* 2a */ 3, /* 2b */ 4,
	/* 2c */ 3, /* 2d */ 4, /* 2e */ 4, /* 2f */ 5,
	/* 30 */ 2, /* 31 */ 3, /* 32 */ 3, /* 33 */ 4,
	/* 34 */ 3, /* 35 */ 4, /* 36 */ 4, /* 37 */ 5,
	/* 38 */ 3, /* 39 */ 4, /* 3a */ 4, /* 3b */ 5,
	/* 3c */ 4, /* 3d */ 5, /* 3e */ 5, /* 3f */ 6,
	/* 40 */ 1, /* 41 */ 2, /* 42 */ 2, /* 43 */ 3,
	/* 44 */ 2, /* 45 */ 3, /* 46 */ 3, /* 47 */ 4,
	/* 48 */ 2, /* 49 */ 3, /* 4a */ 3, /* 4b */ 4,
	/* 4c */ 3, /* 4d */ 4, /* 4e */ 4, /* 4f */ 5,
	/* 50 */ 2, /* 51 */ 3, /* 52 */ 3, /* 53 */ 4,
	/* 54 */ 3, /* 55 */ 4, /* 56 */ 4, /* 57 */ 5,
	/* 58 */ 3, /* 59 */ 4, /* 5a */ 4, /* 5b */ 5,
	/* 5c */ 4, /* 5d */ 5, /* 5e */ 5, /* 5f */ 6,
	/* 60 */ 2, /* 61 */ 3, /* 62 */ 3, /* 63 */ 4,
	/* 64 */ 3, /* 65 */ 4, /* 66 */ 4, /* 67 */ 5,
	/* 68 */ 3, /* 69 */ 4, /* 6a */ 4, /* 6b */ 5,
	/* 6c */ 4, /* 6d */ 5, /* 6e */ 5, /* 6f */ 6,
	/* 70 */ 3, /* 71 */ 4, /* 72 */ 4, /* 73 */ 5,
	/* 74 */ 4, /* 75 */ 5, /* 76 */ 5, /* 77 */ 6,
	/* 78 */ 4, /* 79 */ 5, /* 7a */ 5, /* 7b */ 6,
	/* 7c */ 5, /* 7d */ 6, /* 7e */ 6, /* 7f */ 7,
	/* 80 */ 1, /* 81 */ 2, /* 82 */ 2, /* 83 */ 3,
	/* 84 */ 2, /* 85 */ 3, /* 86 */ 3, /* 87 */ 4,
	/* 88 */ 2, /* 89 */ 3, /* 8a */ 3, /* 8b */ 4,
	/* 8c */ 3, /* 8d */ 4, /* 8e */ 4, /* 8f */ 5,
	/* 90 */ 2, /* 91 */ 3, /* 92 */ 3, /* 93 */ 4,
	/* 94 */ 3, /* 95 */ 4, /* 96 */ 4, /* 97 */ 5,
	/* 98 */ 3, /* 99 */ 4, /* 9a */ 4, /* 9b */ 5,
	/* 9c */ 4, /* 9d */ 5, /* 9e */ 5, /* 9f */ 6,
	/* a0 */ 2, /* a1 */ 3, /* a2 */ 3, /* a3 */ 4,
	/* a4 */ 3, /* a5 */ 4, /* a6 */ 4, /* a7 */ 5,
	/* a8 */ 3, /* a9 */ 4, /* aa */ 4, /* ab */ 5,
	/* ac */ 4, /* ad */ 5, /* ae */ 5, /* af */ 6,
	/* b0 */ 3, /* b1 */ 4, /* b2 */ 4, /* b3 */ 5,
	/* b4 */ 4, /* b5 */ 5, /* b6 */ 5, /* b7 */ 6,
	/* b8 */ 4, /* b9 */ 5, /* ba */ 5, /* bb */ 6,
	/* bc */ 5, /* bd */ 6, /* be */ 6, /* bf */ 7,
	/* c0 */ 2, /* c1 */ 3, /* c2 */ 3, /* c3 */ 4,
	/* c4 */ 3, /* c5 */ 4, /* c6 */ 4, /* c7 */ 5,
	/* c8 */ 3, /* c9 */ 4, /* ca */ 4, /* cb */ 5,
	/* cc */ 4, /* cd */ 5, /* ce */ 5, /* cf */ 6,
	/* d0 */ 3, /* d1 */ 4, /* d2 */ 4, /* d3 */ 5,
	/* d4 */ 4, /* d5 */ 5, /* d6 */ 5, /* d7 */ 6,
	/* d8 */ 4, /* d9 */ 5, /* da */ 5, /* db */ 6,
	/* dc */ 5, /* dd */ 6, /* de */ 6, /* df */ 7,
	/* e0 */ 3, /* e1 */ 4, /* e2 */ 4, /* e3 */ 5,
	/* e4 */ 4, /* e5 */ 5, /* e6 */ 5, /* e7 */ 6,
	/* e8 */ 4, /* e9 */ 5, /* ea */ 5, /* eb */ 6,
	/* ec */ 5, /* ed */ 6, /* ee */ 6, /* ef */ 7,
	/* f0 */ 4, /* f1 */ 5, /* f2 */ 5, /* f3 */ 6,
	/* f4 */ 5, /* f5 */ 6, /* f6 */ 6, /* f7 */ 7,
	/* f8 */ 5, /* f9 */ 6, /* fa */ 6, /* fb */ 7,
	/* fc */ 6, /* fd */ 7, /* fe */ 7, /* ff */ 8
};

uint32_t c_popcount(uint8_t* buffer, int chunks16) {
	uint32_t dummy __attribute__((unused));
	uint32_t n = 0;

#if 0
	int i;
	for (i=0; i < chunks16 * 16; i+=4) {
		n += POPCOUNT_8bit[buffer[i+0]];
		n += POPCOUNT_8bit[buffer[i+1]];
		n += POPCOUNT_8bit[buffer[i+2]];
		n += POPCOUNT_8bit[buffer[i+3]];
	}
#endif
	__asm__ volatile (
		"	xor %%eax, %%eax		\n"
		"0:					\n"
		"	movl (%%esi), %%ebx		\n"
		"	addl      $4, %%esi		\n"
		"					\n"
		"	movzbl  %%bl, %%edx		\n"
		"	movzbl  %%bh, %%edi		\n"
		"	addl POPCOUNT_8bit(,%%edx,4), %%eax	\n"
		"	addl POPCOUNT_8bit(,%%edi,4), %%eax	\n"

		"	shrl     $16, %%ebx		\n"
		"	subl      $1, %%ecx		\n"

		"	movzbl  %%bl, %%edx		\n"
		"	movzbl  %%bh, %%edi		\n"
		"	addl POPCOUNT_8bit(,%%edx,4), %%eax	\n"
		"	addl POPCOUNT_8bit(,%%edi,4), %%eax	\n"
		"					\n"
		"	test  %%ecx, %%ecx		\n"
		"	jnz   0b			\n"
		: "=a" (n),
		  "=S" (dummy),
		  "=c" (dummy)
		: "c" (chunks16 * 16 / 4),
		  "S" (buffer)
		: "ebx", "edx", "edi"
	);

	return n;
}

// ---- SSE2 - naive approach ---------------------------------------------
uint32_t sse2_popcount1(uint8_t* buffer, int chunks16) {
	static uint8_t mask33[16] = {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}; 
	static uint8_t mask55[16] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}; 
	static uint8_t mask0f[16] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f}; 

	uint32_t result = 0;
	uint32_t tmp;
	int n;

	__asm__ volatile ("movdqu (%%eax), %%xmm5" : : "a" (mask55));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (mask33));
	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (mask0f));

	for (n=0; n < chunks16; n++) {
		__asm__ volatile(
#if ALIGN_DATA
			"movdqa	  (%%ebx), %%xmm0	\n"
#else
			"movdqu	  (%%ebx), %%xmm0	\n"
#endif
			"movdqa    %%xmm0, %%xmm1	\n"
			"psrlw         $1, %%xmm1	\n"
			"pand      %%xmm5, %%xmm0	\n"
			"pand      %%xmm5, %%xmm1	\n"
			"paddb     %%xmm1, %%xmm0	\n"

			"movdqa    %%xmm0, %%xmm1	\n"
			"psrlw         $2, %%xmm1	\n"
			"pand      %%xmm6, %%xmm0	\n"
			"pand      %%xmm6, %%xmm1	\n"
			"paddb     %%xmm1, %%xmm0	\n"

			"movdqa    %%xmm0, %%xmm1	\n"
			"psrlw         $4, %%xmm1	\n"
			"pand      %%xmm7, %%xmm0	\n"
			"pand      %%xmm7, %%xmm1	\n"
			"paddb     %%xmm1, %%xmm0	\n"

			"pxor      %%xmm1, %%xmm1	\n"	// popcount for all bytes
			"psadbw    %%xmm1, %%xmm0	\n"	// sum popcounts

			"movhlps   %%xmm0, %%xmm1	\n"
			"paddd     %%xmm0, %%xmm1	\n"
			"movd      %%xmm1, %%eax	\n"

			: "=a" (tmp)
			: "b" (&buffer[n*16])
		);
		result += tmp;
	}

	return result;
}

// ---- SSE2 - naive approach improved ------------------------------------
uint32_t sse2_popcount2(uint8_t* buffer, int chunks16) {
	static uint8_t mask33[16] = {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}; 
	static uint8_t mask55[16] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}; 
	static uint8_t mask0f[16] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f}; 

	uint32_t result = 0;
	int i, n, k;

	__asm__ volatile ("movdqu (%%eax), %%xmm5" : : "a" (mask55));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (mask33));
	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (mask0f));
	__asm__ volatile ("pxor %xmm4, %xmm4");	// global accumulator

	i = 0;
	while (chunks16 > 0) {
		// max(POPCOUNT_8bit) = 8, thus byte-wise addition could be done
		// for floor(255/8) = 31 iterations
		if (chunks16 > 31) {
			k = 31;
			chunks16 -= 31;
		}
		else {
			k = chunks16;
			chunks16 = 0;
		}

		__asm__ volatile ("pxor %xmm3, %xmm3"); // xmm3 -- local accumulator
		for (n=0; n < k; n++) {
			__asm__ volatile(
#if ALIGN_DATA
				"movdqa	  (%%eax), %%xmm0	\n"
#else
				"movdqu	  (%%eax), %%xmm0	\n"
#endif
				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $1, %%xmm1	\n"
				"pand      %%xmm5, %%xmm0	\n"
				"pand      %%xmm5, %%xmm1	\n"
				"paddb     %%xmm1, %%xmm0	\n"

				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $2, %%xmm1	\n"
				"pand      %%xmm6, %%xmm0	\n"
				"pand      %%xmm6, %%xmm1	\n"
				"paddb     %%xmm1, %%xmm0	\n"

				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $4, %%xmm1	\n"
				"pand      %%xmm7, %%xmm0	\n"
				"pand      %%xmm7, %%xmm1	\n"
				"paddb     %%xmm1, %%xmm0	\n"

				"paddb     %%xmm0, %%xmm3	\n"	// update local accumulator

				: 
				: "a" (&buffer[i])
			);
			i += 16;
		}

		// update global accumulator (two 32-bits counters)
		__asm__ volatile (
			"pxor	%xmm0, %xmm0		\n"
			"psadbw	%xmm0, %xmm3		\n"
			"paddd	%xmm3, %xmm4		\n"
		);
	}

	// finally add together 32-bits counters stored in global accumulator
	__asm__ volatile (
		"movhlps   %%xmm4, %%xmm0	\n"
		"paddd     %%xmm4, %%xmm0	\n"
		"movd      %%xmm0, %%eax	\n"
		: "=a" (result)
	);

	return result;
}


// ---- SSE2 - naive approach improved - inner loop unrolled --------------
uint32_t sse2_popcount3(uint8_t* buffer, int chunks16) {
	static uint8_t mask33[16] = {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}; 
	static uint8_t mask55[16] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}; 
	static uint8_t mask0f[16] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f}; 

	uint32_t result = 0;
	int i, n, k;

#ifdef DEBUG
	assert(chunks % 4 == 0)
#ifndef ALIGN_DATA
	assert(false);
#endif
#endif

	__asm__ volatile ("movdqu (%%eax), %%xmm5" : : "a" (mask55));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (mask33));
	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (mask0f));
	__asm__ volatile ("pxor %xmm4, %xmm4");	// global accumulator

	i = 0;
	while (chunks16 > 0) {
#define MAX (7*4)
		if (chunks16 > MAX) {
			k = MAX;
			chunks16 -= MAX;
		}
		else {
			k = chunks16;
			chunks16 = 0;
		}
#undef MAX

		__asm__ volatile ("pxor %xmm3, %xmm3"); // xmm3 -- local accumulator
		for (n=0; n < k; n += 4) {
#define block(index) \
			__asm__ volatile( \
				"movdqa	  (%%eax), %%xmm0	\n" \
				"movdqa    %%xmm0, %%xmm1	\n" \
				"psrlw         $1, %%xmm1	\n" \
				"pand      %%xmm5, %%xmm0	\n" \
				"pand      %%xmm5, %%xmm1	\n" \
				"paddb     %%xmm1, %%xmm0	\n" \
				"movdqa    %%xmm0, %%xmm1	\n" \
				"psrlw         $2, %%xmm1	\n" \
				"pand      %%xmm6, %%xmm0	\n" \
				"pand      %%xmm6, %%xmm1	\n" \
				"paddb     %%xmm1, %%xmm0	\n" \
				"movdqa    %%xmm0, %%xmm1	\n" \
				"psrlw         $4, %%xmm1	\n" \
				"pand      %%xmm7, %%xmm0	\n" \
				"pand      %%xmm7, %%xmm1	\n" \
				"paddb     %%xmm1, %%xmm0	\n" \
				"paddb     %%xmm0, %%xmm3	\n"	\
				: : "a" (&buffer[index]));

			block(i + 0*16);
			block(i + 1*16);
			block(i + 2*16);
			block(i + 3*16);
			i += 4*16;
		}

		// update global accumulator (two 32-bits counters)
		__asm__ volatile (
			"pxor	%xmm0, %xmm0		\n"
			"psadbw	%xmm0, %xmm3		\n"
			"paddd	%xmm3, %xmm4		\n"
		);
	}

	// finally add together 32-bits counters stored in global accumulator
	__asm__ volatile (
		"movhlps   %%xmm4, %%xmm0	\n"
		"paddd     %%xmm4, %%xmm0	\n"
		"movd      %%xmm0, %%eax	\n"
		: "=a" (result)
	);

	return result;
}


// ---- SSSE3 - naive approach --------------------------------------------
uint32_t ssse3_popcount1(uint8_t* buffer, int chunks16) {
	uint32_t dummy __attribute__((unused));
	static char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};

	uint32_t result, tmp;
	int n;

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));

	result = 0;
	for (n=0; n < chunks16; n++) {
		__asm__ volatile(
#if ALIGN_DATA
			"movdqa	  (%%ebx), %%xmm0	\n"
#else
			"movdqu	  (%%ebx), %%xmm0	\n"
#endif
			"movdqa    %%xmm0, %%xmm1	\n"

			"psrlw         $4, %%xmm1	\n"
			"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
			"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
			"movdqa    %%xmm7, %%xmm2	\n"
			"movdqa    %%xmm7, %%xmm3	\n"	// get popcount
			"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
			"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB

			"paddb     %%xmm2, %%xmm3	\n"	// popcount for all bytes
			"pxor      %%xmm0, %%xmm0	\n"
			"psadbw    %%xmm3, %%xmm0	\n"	// sum popcounts

			"movhlps   %%xmm0, %%xmm1	\n"
			"paddd     %%xmm0, %%xmm1	\n"
			"movd      %%xmm1, %%eax	\n"

			: "=a" (tmp)
			: "b" (&buffer[n*16])
		);
		result += tmp;
	}

	return result;
}


// ---- SSSE3 - better alorithm, minimized psadbw usage -------------------
uint32_t ssse3_popcount2(uint8_t* buffer, int chunks16) {
	static char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};

	uint32_t result;

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));
	__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // xmm5 -- global accumulator

	result = 0;

	int k, n, i;

	i = 0;
	while (chunks16 > 0) {
		// max(POPCOUNT_8bit) = 8, thus byte-wise addition could be done
		// for floor(255/8) = 31 iterations
#define MAX (31)
		if (chunks16 > MAX) {
			k = MAX;
			chunks16 -= MAX;
		}
		else {
			k = chunks16;
			chunks16 = 0;
		}
#undef MAX

		__asm__ volatile ("pxor %xmm4, %xmm4"); // xmm4 -- local accumulator
		for (n=0; n < k; n++) {
			__asm__ volatile(
#if ALIGN_DATA
				"movdqa	  (%%eax), %%xmm0	\n"
#else
				"movdqu	  (%%eax), %%xmm0	\n"
#endif
				"movdqa    %%xmm0, %%xmm1	\n"

				"psrlw         $4, %%xmm1	\n"
				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles

				"movdqa    %%xmm7, %%xmm2	\n"
				"movdqa    %%xmm7, %%xmm3	\n"	// get popcount
				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB

				"paddb     %%xmm2, %%xmm4	\n"	// update local
				"paddb     %%xmm3, %%xmm4	\n"	// accumulator

				: 
				: "a" (&buffer[i])
			);
			i += 16;
		}

		// update global accumulator (two 32-bits counters)
		__asm__ volatile (
			"pxor	%xmm0, %xmm0		\n"
			"psadbw	%xmm0, %xmm4		\n"
			"paddd	%xmm4, %xmm5		\n"
		);
	}

	// finally add together 32-bits counters stored in global accumulator
	__asm__ volatile (
		"movhlps   %%xmm5, %%xmm0	\n"
		"paddd     %%xmm5, %%xmm0	\n"
		"movd      %%xmm0, %%eax	\n"
		: "=a" (result)
	);

	return result;
}

// ---- SSSE3 - better alorithm, inner loop unrolled ----------------------
uint32_t ssse3_popcount3(uint8_t* buffer, int chunks16) {
	static char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};

	uint32_t result;

#ifdef DEBUG
	assert(chunks16 % 4 == 0);
#endif

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));
	__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // xmm5 -- global accumulator

	result = 0;

	int k, n, i;

	i = 0;
	while (chunks16 > 0) {
		// max(POPCOUNT_8bit) = 8, thus byte-wise addition could be done
		// for floor(255/8) = 31 iterations
#define MAX (7*4)
		if (chunks16 > MAX) {
			k = MAX;
			chunks16 -= MAX;
		}
		else {
			k = chunks16;
			chunks16 = 0;
		}
#undef MAX
		__asm__ volatile ("pxor %xmm4, %xmm4"); // xmm4 -- local accumulator
		for (n=0; n < k; n+=4) {
#define body(index) \
			__asm__ volatile( \
				"movdqa	  (%%eax), %%xmm0	\n" \
				"movdqa    %%xmm0, %%xmm1	\n" \
				"psrlw         $4, %%xmm1	\n" \
				"pand      %%xmm6, %%xmm0	\n" \
				"pand      %%xmm6, %%xmm1	\n" \
				"movdqa    %%xmm7, %%xmm2	\n" \
				"movdqa    %%xmm7, %%xmm3	\n" \
				"pshufb    %%xmm0, %%xmm2	\n" \
				"pshufb    %%xmm1, %%xmm3	\n" \
				"paddb     %%xmm2, %%xmm4	\n" \
				"paddb     %%xmm3, %%xmm4	\n" \
				: : "a" (&buffer[index]));

			body(i);
			body(i + 1*16);
			body(i + 2*16);
			body(i + 3*16);
#undef body
			i += 4*16;
		}

		// update global accumulator (two 32-bits counters)
		__asm__ volatile (
			"pxor	%xmm0, %xmm0		\n"
			"psadbw	%xmm0, %xmm4		\n"
			"paddd	%xmm4, %xmm5		\n"
		);
	}

	// finally add together 32-bits counters stored in global accumulator
	__asm__ volatile (
		"movhlps   %%xmm5, %%xmm0	\n"
		"paddd     %%xmm5, %%xmm0	\n"
		"movd      %%xmm0, %%eax	\n"
		: "=a" (result)
	);

	return result;
}


#define OPT_COUNT 8

char* functions[OPT_COUNT] = {
	"verify",
	"lookup",
	"sse2-1",
	"sse2-2",
	"ssse3-1",
	"ssse3-2",
	"ssse3-unrl",
	"sse2-unrl"
};


void help(const char* progname) {
	int i;
	printf("%s ", progname);

	printf("%s", functions[0]);
	for (i=1; i < OPT_COUNT; i++)
		printf("|%s", functions[i]);

	printf(" 16-byte-chunks repeat-count\n");
	exit(1);
}


int main(int argc, char* argv[]) {
	// prog parametrs
	int function;
	int chunks_count;
	int repeat_count;
	int default_chunks_count = 100;
	int default_repeat_count = 100000;

	int i;

	// parse arguments
	if (argc == 1)
		help(argv[0]);
	// - function
	for (function=0; function < OPT_COUNT; function++)
		if (strcasecmp(argv[1], functions[function]) == 0)
			break;
	
	if (function == OPT_COUNT)
		help(argv[0]);
	
	// - 16-byte chunks
	if (argc >= 3) {
		chunks_count = atoi(argv[2]);
		if (chunks_count <= 0 || chunks_count > MAX_CHUNKS)
			help(argv[0]);
	}
	else
		chunks_count = default_chunks_count;
	
	// - repeat count
	if (argc >= 4) {
		repeat_count = atoi(argv[3]);
		if (repeat_count <= 0)
			help(argv[0]);
	}
	else
		repeat_count = default_repeat_count;


	// fill buffer with random data
	srand(time(NULL));
	for (i=0; i < sizeof(buffer); i++)
		buffer[i] = rand() % 256;


	// run
	printf("action=%s, chunks=%d, repeat count=%d\n",
		functions[function], chunks_count, repeat_count);

	uint32_t popcount_ref, popcount;
	char failed = 0;
	switch (function) {
		case 0:
			popcount_ref = c_popcount(buffer, chunks_count);

			// lookup result is reference
			printf("%10s -> %d\n", functions[1], popcount_ref);

#define verify(index, function) \
			popcount = function(buffer, chunks_count); \
			printf("%10s -> %d %s\n", \
				functions[index], \
				popcount, \
				(popcount_ref != popcount) ? "FAILED!!!" : "" \
			); \
			if (popcount_ref != popcount) \
				failed = 1;

			verify(2, sse2_popcount1);
			verify(3, sse2_popcount2);
			verify(4, ssse3_popcount1);
			verify(5, ssse3_popcount2);
			verify(6, ssse3_popcount3);
			verify(7, sse2_popcount3);

			if (failed)
				return EXIT_FAILURE;

			break;

		case 1:
			while (repeat_count--)
				c_popcount(buffer, chunks_count);
			break;

		case 2:
			while (repeat_count--)
				sse2_popcount1(buffer, chunks_count);
			break;

		case 3:
			while (repeat_count--)
				sse2_popcount2(buffer, chunks_count);
			break;

		case 4:
			while (repeat_count--)
				ssse3_popcount1(buffer, chunks_count);
			break;

		case 5:
			while (repeat_count--)
				ssse3_popcount2(buffer, chunks_count);
			break;

		case 6:
			while (repeat_count--)
				ssse3_popcount3(buffer, chunks_count);
			break;

		case 7:
			while (repeat_count--)
				sse2_popcount3(buffer, chunks_count);
			break;
	}

	return EXIT_SUCCESS;
}

// eof
