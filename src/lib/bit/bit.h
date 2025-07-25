#ifndef TARANTOOL_LIB_BIT_BIT_H_INCLUDED
#define TARANTOOL_LIB_BIT_BIT_H_INCLUDED
/*
 * Copyright 2010-2016, Tarantool AUTHORS, please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * @file
 * @brief Bit manipulation library
 */
#include "trivia/util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#if defined(HAVE_FFSL) || defined(HAVE_FFSLL)
#include <strings.h>
#endif /* defined(HAVE_FFSL) || defined(HAVE_FFSLL) */
#include <limits.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/**
 * @brief Unaligned load from memory.
 * @param p pointer
 * @return number
 */
inline uint8_t
load_u8(const void *p)
{
	uint8_t res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline uint16_t
load_u16(const void *p)
{
	uint16_t res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline uint32_t
load_u32(const void *p)
{
	uint32_t res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline uint64_t
load_u64(const void *p)
{
	uint64_t res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline float
load_float(const void *p)
{
	float res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline double
load_double(const void *p)
{
	double res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/** @copydoc load_u8 */
inline bool
load_bool(const void *p)
{
	bool res;
	memcpy(&res, p, sizeof(res));
	return res;
}

/**
 * @brief Unaligned store to memory.
 * @param p pointer
 * @param v number
 */
inline void
store_u8(void *p, uint8_t v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_u8 */
inline void
store_u16(void *p, uint16_t v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_u8 */
inline void
store_u32(void *p, uint32_t v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_u8 */
inline void
store_u64(void *p, uint64_t v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_u8 */
inline void
store_float(void *p, float v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_u8 */
inline void
store_double(void *p, double v)
{
	memcpy(p, &v, sizeof(v));
}

/** @copydoc store_bool */
inline void
store_bool(void *p, bool v)
{
	memcpy(p, &v, sizeof(v));
}

/**
 * @brief Returns the size of memory needed to store a bitmap
 * of \a bit_count bits.
 * The function rounds the size up to a multiple of the word
 * size, which is required by bit_set() and bit_clear().
 * @param bit_count number of bits in the bitmap
 * @retval bitmap size, in bytes
 */
#define BITMAP_SIZE(bit_count) \
	(DIV_ROUND_UP((bit_count), CHAR_BIT * sizeof(long)) * sizeof(long))

/**
 * @brief Test bit \a pos in memory chunk \a data
 * @param data memory chunk
 * @param pos bit number (zero-based)
 * @retval true bit \a pos is set in \a data
 * @retval false otherwise
 */
inline bool
bit_test(const void *data, size_t pos)
{
	size_t chunk = pos / CHAR_BIT;
	size_t offset = pos % CHAR_BIT;

	const uint8_t *ldata = (const uint8_t *)data;
	return (ldata[chunk] >> offset) & 0x1;
}

/**
 * @brief Set bit \a pos in a memory chunk \a data
 * @param data memory chunk
 * @param pos bit number (zero-based)
 * @return previous value
 * @see bit_test
 * @see bit_clear
 */
inline bool
bit_set(void *data, size_t pos)
{
	size_t chunk = pos / CHAR_BIT;
	size_t offset = pos % CHAR_BIT;

	uint8_t *ldata = (uint8_t *)data;
	bool prev = (ldata[chunk] >> offset) & 0x1;
	ldata[chunk] |= (1UL << offset);
	return prev;
}

/**
 * @brief Clear bit \a pos in memory chunk \a data
 * @param data memory chunk
 * @param pos bit number (zero-based)
 * @return previous value
 * @see bit_test
 * @see bit_set
 */
inline bool
bit_clear(void *data, size_t pos)
{
	size_t chunk = pos / CHAR_BIT;
	size_t offset = pos % CHAR_BIT;

	uint8_t *ldata = (uint8_t *)data;
	bool prev = (ldata[chunk] >> offset) & 0x1;
	ldata[chunk] &= ~(1UL << offset);
	return prev;
}

/**
 * @brief Set \a count bits in a memory chunk \a data starting from bit \a pos
 *  to the value \a val.
 * @param data - memory chunk
 * @param pos - the first bit number (zero-based)
 * @param count - the amount of bits to set
 * @param val - the new value of bits
 */
inline void
bit_set_range(void *data, size_t pos, size_t count, bool val)
{
	if (count == 0)
		return;
	/*
	 * We can have:
	 *  - a head of bits in the start;
	 *  - a bunch of whole bytes in the middle;
	 *  - a tail of bits in the end.
	 */
	size_t pos_byte = pos / CHAR_BIT;
	size_t pos_bit = pos % CHAR_BIT;
	/*
	 * The head may be the only byte to copy to.
	 */
	size_t head_size = pos_bit + count < CHAR_BIT ?
			   count : CHAR_BIT - pos_bit;
	size_t rest_size = count - head_size;    /* Can be 0. */
	size_t body_size = rest_size / CHAR_BIT; /* In bytes. */
	size_t tail_size = rest_size % CHAR_BIT; /* In bits. */

	size_t head_mask = ((1ULL << head_size) - 1) << pos_bit;
	size_t tail_mask = ((1ULL << tail_size) - 1);

	uint8_t value = val ? 0xFF : 0x00;
	uint8_t *head = (uint8_t *)data + pos_byte;
	uint8_t *body = head + 1;
	uint8_t *tail = body + body_size;
	/*
	 * Set the head bits.
	 */
	*head = (*head & ~head_mask) | (value & head_mask);
	/*
	 * Set the body bytes.
	 */
	memset(body, value, body_size);
	/*
	 * Set the tail bits. The if-statement is required to avoid overwriting
	 * a byte beyond the buffer even if `tail_mask' is empty.
	 */
	if (tail_size > 0)
		*tail = (*tail & ~tail_mask) | (value & tail_mask);
	/*
	 * Once you are done trying to "optimize" this routine, and have
	 * realized what a terrible mistake that was, please increment
	 * the following counter as a warning to the next guy:
	 *
	 * total_hours_wasted_here = 8
	 */
}

/**
 * @brief Copy \a count bits from memory chunk \a src starting from bit \a src_i
 *  into \a dst starting from bit \a dst_i.
 * @param dst - the memory chunk to copy bits into.
 * @param dst_i - the position to copy bits into.
 * @param count - the amount of bits to copy.
 * @param src - the memory chunk to copy bits from.
 * @param src_i - the position to copy bits from.
 * @pre the bit buffers do not overlap.
 */
static inline void
bit_copy_range(uint8_t *restrict dst, size_t dst_i, const uint8_t *restrict src,
	       size_t src_i, size_t count)
{
	if (count == 0)
		return;

	/* Assure the buffers are non-overlapping. */
	assert(src > dst + DIV_ROUND_UP(dst_i + count, CHAR_BIT) - 1 ||
	       dst > src + DIV_ROUND_UP(src_i + count, CHAR_BIT) - 1);

	/*
	 * We can have:
	 * - a head of bits in the start;
	 * - a bunch of whole bytes in the middle;
	 * - a tail of bits in the end.
	 */
	size_t dst_i_byte = dst_i / CHAR_BIT;
	size_t dst_i_bit = dst_i % CHAR_BIT;
	size_t src_i_byte = src_i / CHAR_BIT;
	size_t src_i_bit = src_i % CHAR_BIT;

	/* We select shift directions based on this. */
	ssize_t diff_bit = dst_i_bit - src_i_bit;

	/* The head may be the only byte to copy to. */
	size_t dst_head_size = dst_i_bit + count < CHAR_BIT ?
			       count : CHAR_BIT - dst_i_bit;
	size_t dst_rest_size = count - dst_head_size;    /* Can be 0. */
	size_t dst_body_size = dst_rest_size / CHAR_BIT; /* In bytes. */
	size_t dst_tail_size = dst_rest_size % CHAR_BIT; /* In bits. */

	/*
	 *    dst_i_bit
	 *        |
	 * Dst: - D D D - - - -
	 *        \___/
	 *          |
	 *    dst_head_mask (but in most cases the head is until end of byte)
	 */
	size_t dst_head_mask = ((1 << dst_head_size) - 1) << dst_i_bit;
	size_t dst_tail_mask = (1 << dst_tail_size) - 1;

	if (diff_bit <= 0) {
		/*
		 *      Head              Body              Tail
		 * Dst: - - - D D D D D   D D D D D D D D   D - - - - - - -
		 * Src: - - - - - S S S   S S S S S S S S   S S S - - - - -
		 *            \_/         \_/ \_________/
		 *             |           |       |
		 *           shift       shift  shift_in
		 */
		size_t shift = -diff_bit;
		size_t shift_in = CHAR_BIT - shift;

		/*
		 * We're copying offsetted data from src to dst. So, in general,
		 * we shift the source left for `shift` bits, then we take first
		 * `shift` bits of the next source byte in. Given example above
		 * (note the shift right moves bits left since they're written
		 * from the least to the most significant):
		 *
		 * Src[0]: [- - - - - 0 1 2] >> shift    = [- - - 0 1 2 - -]
		 * Src[1]: [3 4 5 6 7 8 9 a] << shift_in = [- - - - - - 3 4]
		 * Combine the two into result to write:   [- - - 0 1 2 3 4]
		 *
		 * Effectively we combine `shift_in` last bits of Src[0] and
		 * `shift` first bits of Src[1]. But we can only do that if we
		 * have anything to shift in. It's totally possible to have a
		 * situation like this:
		 *
		 * Dst: - - - D D D - -
		 * Src: - - - - - S S S
		 *
		 * Here we only have one byte of source and should not attempt
		 * to read next ones, since all the source bits required are
		 * located in the single byte. In this case we just shift the
		 * bits of the first source byte right to align them with Dst.
		 *
		 * So we have to check if we have next bytes to shift first bits
		 * from in order to prevent OOB reads.
		 */
		unsigned src_0 = src[src_i_byte];
		/* The source is at least 2 bytes? Can read the second byte. */
		bool can_read_src1 = src_i_bit + count > CHAR_BIT;
		unsigned src_1 = can_read_src1 ? src[src_i_byte + 1] : 0;
		/* Copy the head bits. */
		dst[dst_i_byte] = (dst[dst_i_byte] & ~dst_head_mask) |
				  (((src_0 >> shift) | (src_1 << shift_in)) &
				   dst_head_mask);

		/* Copy the body bytes. */
		for (size_t i = 0; i < dst_body_size; i++) {
			size_t dst_curr = dst_i_byte + 1 + i;
			size_t src_curr = src_i_byte + 1 + i;
			unsigned src_0 = src[src_curr];
			/*
			 * If we have a non-zero shift, we must have the next
			 * byte available to shift-in first bits from.
			 */
			bool can_read_src1 = shift != 0;
			unsigned src_1 = can_read_src1 ? src[src_curr + 1] : 0;
			dst[dst_curr] = (src_0 >> shift) | (src_1 << shift_in);
		}

		/* Copy the tail bits. */
		if (dst_tail_size > 0) {
			size_t dst_curr = dst_i_byte + 1 + dst_body_size;
			size_t src_curr = src_i_byte + 1 + dst_body_size;

			/*
			 * We have the destination tail, so if we access the
			 * next byte after the one we're currently on, we can
			 * run out of bounds. Let's see when that happens:
			 *
			 * Dst: - - - D D D D D   D D D D D - - -
			 * Src: - - - - - S S S   S S S S S S S -
			 *
			 * Here we can't read the next byte cause all the bits
			 * we need are located in the last byte of the source.
			 * But we can have something like this:
			 *
			 *      Head            Tail            Next byte
			 * Dst: - D D D D D D D D D D D D - - - - - - - - - - -
			 *                                \___/
			 *                                  |
			 *                      (CHAR_BIT - dst_tail_size)
			 *
			 * Src: - - - - - S S S S S S S S S S S S - - - - - - -
			 *        \_____/
			 *           |
			 *         shift
			 *
			 * Here the source bits we need are located in the next
			 * source byte. This happens when the amount of non-dst
			 * bits in the tail is less than `shift`.
			 */
			bool can_read_src1 = shift > (CHAR_BIT - dst_tail_size);
			unsigned src_0 = src[src_curr];
			unsigned src_1 = can_read_src1 ? src[src_curr + 1] : 0;
			dst[dst_curr] = (dst[dst_curr] & ~dst_tail_mask) |
					(((src_0 >> shift) |
					  (src_1 << shift_in)) &
					 dst_tail_mask);
		}
	} else {
		/*
		 *      Head              Body              Tail
		 * Dst: - - - - - D D D   D D D D D D D D   D D D - - - - -
		 *            \_/
		 *             |
		 *           shift
		 *
		 *       shift_in
		 *       ____|____
		 *      /         \
		 * Src: - - - S S S S S   S S S S S S S S   S - - - - - - -
		 *            \_/   \_/
		 *             |     |
		 *           shift  carry
		 */
		size_t shift = diff_bit;
		size_t shift_in = CHAR_BIT - shift;

		unsigned src_0 = src[src_i_byte];
		/* Copy the head bits. */
		dst[dst_i_byte] = (dst[dst_i_byte] & ~dst_head_mask) |
				  ((src_0 << shift) & dst_head_mask);

		/* Copy the body bytes. */
		for (size_t i = 0; i < dst_body_size; i++) {
			size_t dst_curr = dst_i_byte + 1 + i;
			size_t src_curr = src_i_byte + 1 + i;
			unsigned carry = src[src_curr - 1] >> shift_in;
			unsigned src_0 = src[src_curr];
			dst[dst_curr] = carry | (src_0 << shift);
		}

		/* Copy the tail bits. */
		if (dst_tail_size > 0) {
			size_t dst_curr = dst_i_byte + 1 + dst_body_size;
			size_t src_curr = src_i_byte + 1 + dst_body_size;
			unsigned carry = src[src_curr - 1] >> shift_in;

			/*
			 * It may so happen that the amount of bytes the source
			 * is scattered over is smaller than the destination, in
			 * this case we can't read the next source byte because
			 * we can go out of the source buffer bounds, e. g.:
			 *
			 *      Head            Body            Tail
			 * Dst: - - - - - - D D D D D D D D D D D D - - - - - -
			 * Src: - - - S S S S S S S S S S S S - - - - - - - - -
			 *                                \___/ \_____________/
			 *                                  |          |
			 *                           carry -+    Out of bounds
			 *
			 * This condition may be simplified: this only happens
			 * when we shift for the tail size or more.
			 */
			bool can_read_last = shift < dst_tail_size;
			unsigned src_0 = can_read_last ? src[src_curr] : 0;
			dst[dst_curr] = (dst[dst_curr] & ~dst_tail_mask) |
					((carry | (src_0 << shift)) &
					 dst_tail_mask);
		}
	}
}

/**
 * @cond false
 * @brief Naive implementation of ctz.
 */
#define CTZ_NAIVE(x, bitsize) {						\
	if (x == 0) {							\
		return (bitsize);					\
	}								\
									\
	int r = 0;							\
	for (; (x & 1) == 0; r++) {					\
		x >>= 1;						\
	}								\
									\
	return r;							\
}
/** @endcond */

/**
 * @brief Count Trailing Zeros.
 * Returns the number of trailing 0-bits in @a x, starting at the least
 * significant bit position. If @a x is 0, the result is undefined.
 * @param x integer
 * @see __builtin_ctz()
 * @return the number trailing 0-bits
 */
inline int
bit_ctz_u32(uint32_t x)
{
#if defined(HAVE_BUILTIN_CTZ)
	return __builtin_ctz(x);
#elif defined(HAVE_FFSL)
	return ffsl(x) - 1;
#else
	CTZ_NAIVE(x, sizeof(uint32_t) * CHAR_BIT);
#endif
}

/**
 * @copydoc bit_ctz_u32
 */
inline int
bit_ctz_u64(uint64_t x)
{
#if   defined(HAVE_BUILTIN_CTZLL)
	return __builtin_ctzll(x);
#elif defined(HAVE_FFSLL)
	return ffsll(x) - 1;
#else
	CTZ_NAIVE(x, sizeof(uint64_t) * CHAR_BIT);
#endif
}

#undef CTZ_NAIVE

/**
 * @cond false
 * @brief Naive implementation of clz.
 */
#define CLZ_NAIVE(x, bitsize) {						\
	if (x == 0) {							\
		return  (bitsize);					\
	}								\
									\
	int r = (bitsize);						\
	for (; x; r--) {						\
		x >>= 1;						\
	}								\
									\
	return r;							\
}
/** @endcond */

/**
 * @brief Count Leading Zeros.
 * Returns the number of leading 0-bits in @a x, starting at the most
 * significant bit position. If @a x is 0, the result is undefined.
 * @param x integer
 * @see __builtin_clz()
 * @return the number of leading 0-bits
 */
inline int
bit_clz_u32(uint32_t x)
{
#if   defined(HAVE_BUILTIN_CLZ)
	return __builtin_clz(x);
#else /* !defined(HAVE_BUILTIN_CLZ) */
	CLZ_NAIVE(x, sizeof(uint32_t) * CHAR_BIT);
#endif
}

/**
 * @copydoc bit_clz_u32
 */
inline int
bit_clz_u64(uint64_t x)
{
#if   defined(HAVE_BUILTIN_CLZLL)
	return __builtin_clzll(x);
#else /* !defined(HAVE_BUILTIN_CLZLL) */
	CLZ_NAIVE(x, sizeof(uint64_t) * CHAR_BIT);
#endif
}

#undef CLZ_NAIVE

/**
 * @cond false
 * @brief Naive implementation of popcount.
 */
#define POPCOUNT_NAIVE(x, bitsize)  {					\
	int r;								\
	for (r = 0; x; r++) {						\
		x &= (x-1);						\
	}								\
									\
	return r;							\
}
/** @endcond */

/**
 * @brief Returns the number of 1-bits in @a x.
 * @param x integer
 * @see __builtin_popcount()
 * @return the number of 1-bits in @a x
 */
inline int
bit_count_u32(uint32_t x)
{
#if   defined(HAVE_BUILTIN_POPCOUNT)
	return __builtin_popcount(x);
#else /* !defined(HAVE_BUILTIN_POPCOUNT) */
	POPCOUNT_NAIVE(x, sizeof(uint32_t) * CHAR_BIT);
#endif
}

/**
 * @copydoc bit_count_u32
 */
inline int
bit_count_u64(uint64_t x)
{
#if   defined(HAVE_BUILTIN_POPCOUNTLL)
	return __builtin_popcountll(x);
#else /* !defined(HAVE_BUILTIN_POPCOUNTLL) */
	POPCOUNT_NAIVE(x, sizeof(uint64_t) * CHAR_BIT);
#endif
}

#undef POPCOUNT_NAIVE

/**
 * @brief Returns the number of 1-bits in @a data vector starting from
 *	  @a bit_offset.
 * @param data byte vector of length @a length bits.
 * @param bit_offset offset within the byte vector from where to start
 *		     counting 1-bits.
 * @param length length in bits of byte vector @a data.
 * @return the number of 1-bits in @a data.
 *
 * Implementation adopted from the internals of the Apache
 * Arrow C++ library.
 */
size_t
bit_count(const uint8_t *data, size_t bit_offset, size_t length);

/**
 * @brief Rotate @a x left by @a r bits
 * @param x integer
 * @param r number for bits to rotate
 * @return @a x rotated left by @a r bits
 */
inline uint32_t
bit_rotl_u32(uint32_t x, int r)
{
	assert(r > 0 && r < 32);
	/* gcc recognises this code and generates a rotate instruction */
	return ((x << r) | (x >> (32 - r)));
}

/**
 * @copydoc bit_rotl_u32
 */
inline uint64_t
bit_rotl_u64(uint64_t x, int r)
{
	assert(r > 0 && r < 64);
	/* gcc recognises this code and generates a rotate instruction */
	return ((x << r) | (x >> (64 - r)));
}

/**
 * @copydoc bit_rotl_u32
 */
__attribute__ ((const)) inline uintmax_t
bit_rotl_umax(uintmax_t x, int r)
{
	assert(r > 0 && r < (int)(sizeof(uintmax_t) * CHAR_BIT));
	/* gcc recognises this code and generates a rotate instruction */
	return ((x << r) | (x >> (sizeof(uintmax_t) * CHAR_BIT - r)));
}
/**
 * @brief Rotate @a x right by @a r bits
 * @param x integer
 * @param r number for bits to rotate
 * @return @a x rotated right by @a r bits
 * @todo Move this method to bit.h
 */
inline uint32_t
bit_rotr_u32(uint32_t x, int r)
{
	assert(r > 0 && r < 32);
	/* gcc recognises this code and generates a rotate instruction */
	return ((x >> r) | (x << (32 - r)));
}

/**
 * @copydoc bit_rotr_u32
 */
inline uint64_t
bit_rotr_u64(uint64_t x, int r)
{
	assert(r > 0 && r < 64);
	/* gcc recognises this code and generates a rotate instruction */
	return ((x >> r) | (x << (64 - r)));
}

/**
 * @copydoc bswap_u32
 */
inline uint16_t
bswap_u16(uint16_t x)
{
#if defined(HAVE_BUILTIN_BSWAP16)
	return __builtin_bswap16(x);
#else /* !defined(HAVE_BUILTIN_BSWAP16) */
	return	((x << 8) & UINT16_C(0xff00)) |
		((x >> 8) & UINT16_C(0x00ff));
#endif
}

/**
 * @brief Returns a byte order swapped integer @a x.
 * This function does not take into account host architecture
 * (as it done by htonl / ntohl functions) and always returns @a x
 * with byte order swapped (BE -> LE if @a x is in BE and vice versa).
 * @param x integer
 * @return @a x with swapped bytes
 */
inline uint32_t
bswap_u32(uint32_t x)
{
#if defined(HAVE_BUILTIN_BSWAP32)
	return __builtin_bswap32(x);
#else /* !defined(HAVE_BUILTIN_BSWAP32) */
	return	((x << 24) & UINT32_C(0xff000000)) |
		((x <<  8) & UINT32_C(0x00ff0000)) |
		((x >>  8) & UINT32_C(0x0000ff00)) |
		((x >> 24) & UINT32_C(0x000000ff));
#endif
}

/**
 * @copydoc bswap_u32
 */
inline uint64_t
bswap_u64(uint64_t x)
{
#if defined(HAVE_BUILTIN_BSWAP64)
	return __builtin_bswap64(x);
#else /* !defined(HAVE_BUILTIN_BSWAP64) */
	return  ( (x << 56) & UINT64_C(0xff00000000000000)) |
		( (x << 40) & UINT64_C(0x00ff000000000000)) |
		( (x << 24) & UINT64_C(0x0000ff0000000000)) |
		( (x <<  8) & UINT64_C(0x000000ff00000000)) |
		( (x >>  8) & UINT64_C(0x00000000ff000000)) |
		( (x >> 24) & UINT64_C(0x0000000000ff0000)) |
		( (x >> 40) & UINT64_C(0x000000000000ff00)) |
		( (x >> 56) & UINT64_C(0x00000000000000ff));
#endif
}

/**
 * @brief Index bits in the @a x, i.e. find all positions where bits are set.
 * This method fills @a indexes array with found positions in increasing order.
 * @a offset is added to each index before putting it into @a indexes.
 * @param x integer
 * @param indexes memory array where found indexes are stored
 * @param offset a number added to each index
 * @return pointer to last+1 element in indexes array
 */
int *
bit_index_u32(uint32_t x, int *indexes, int offset);

/**
 * @copydoc bit_index_u32
 */
int *
bit_index_u64(uint64_t x, int *indexes, int offset);

/** @cond false **/
#if defined(__x86_64__)
/* Use bigger words on x86_64 */
#define ITER_UINT uint64_t
#define ITER_CTZ bit_ctz_u64
#define ITER_LOAD load_u64
#else
#define ITER_UINT uint32_t
#define ITER_CTZ bit_ctz_u32
#define ITER_LOAD load_u32
#endif
/** @endcond **/

/**
 * @brief The Bit Iterator
 */
struct bit_iterator {
	/** @cond false **/
	/** Current word to process using ctz **/
	ITER_UINT word;
	/** A bitmask that XORed with word (for set = false iteration) **/
	ITER_UINT word_xor;
	/** A base offset of the word in bits **/
	size_t word_base;
	/** A pointer to the start of a memory chunk **/
	const char *start;
	/** A pointer to the next part of a memory chunk */
	const char *next;
	/** A pointer to the end of a memory chunk */
	const char *end;
	/** @endcond **/
};

/**
 * @brief Initialize bit iterator \a it
 * @param it bit iterator
 * @param data memory chunk
 * @param size size of the memory chunk \a data
 * @param set true to iterate over set bits or false to iterate over clear bits
 */
inline void
bit_iterator_init(struct bit_iterator *it, const void *data, size_t size,
		  bool set)
{
	it->start = (const char *) data;
	it->next = it->start;
	it->end = it->next + size;
	if (unlikely(size == 0)) {
		it->word = 0;
		return;
	}

	it->word_xor = set ? 0 : (ITER_UINT) -1;
	it->word_base = 0;

	/* Check if size is a multiple of sizeof(ITER_UINT) */
	if (likely(size % sizeof(ITER_UINT) == 0)) {
		it->word = ITER_LOAD(it->next);
		it->next += sizeof(ITER_UINT);
	} else {
		const char *e = it->next + size % sizeof(ITER_UINT);
		it->word = it->word_xor;
		char *w = (char *) &it->word;
		while (it->next < e)
			*w++ = *it->next++;
	}
	it->word ^= it->word_xor;
}

/**
 * @brief Return a number of a next set bit in \a it or \a SIZE_MAX
 * if no bits are remain in \a it
 * @param it bit iterator
 * @retval a zero-based number of a next set bit in iterator \a it
 * @retval SIZE_MAX if \a it does not have more set bits
 */
inline size_t
bit_iterator_next(struct bit_iterator *it)
{
	while (unlikely(it->word == 0)) {
		if (unlikely(it->next >= it->end))
			return SIZE_MAX;

		/* Extract the next word from memory */
		it->word = ITER_LOAD(it->next);
		it->word ^= it->word_xor;
		it->word_base = (it->next - it->start) * CHAR_BIT;
		it->next += sizeof(ITER_UINT);
	}

	/* Find the position of a first trailing bit in the current word */
	int bit = ITER_CTZ(it->word);
	/* Remove the first trailing bit from the current word */
	it->word &= it->word - 1;
	/* Add start position if the current word to the found bit */
	return it->word_base + bit;
}

#undef ITER_LOAD
#undef ITER_CTZ
#undef ITER_UINT

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* TARANTOOL_LIB_BIT_BIT_H_INCLUDED */
