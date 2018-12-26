/* Big but self-contained C test program */

#include <stdint.h>

/* C test part 1 - "arm2" - libgcc functions */

volatile uint64_t above = 1;
volatile uint64_t below = 2;
volatile unsigned x1 = 3;
volatile unsigned x2 = 4;
volatile long double t1 = 5.0;
volatile double t2 = 6.0;
volatile float t3 = 7.0f;

void target1 () { above = t1; x1 ++; t1 ++; }
void target2 () { above *= 2; x1 *= 2; t1 *= 3.141; }
void target3 () { below = t2; x2 ++; t2 ++; }
void target4 () { below = t3; x2 *= 2; t2 *= 3.141f; }
void target5 () { above = above / below; }
void target6 () { above = t1; t2 = below; }
void target7 () { t1 = x1 % x2; }
void target8 () { t1 = t2; }
void target9 () { t2 = t1; }
void targeta () { t1 = t1 / t2; }
void targetb () { above = above * below; }

typedef void (* t_target) ();

t_target target_list[] = {
        target1,
        target2,
        target3,
        target4,
        target2,
        target2,
        target2,
        target1,
        target2,
        target4,
        target3,
        target4,
        target5,
        target3,
        target1,
        target2,
        target6,
        target3,
        target7,
        target1,
        target3,
        target8,
        target9,
        targeta,
        targetb,
        (t_target) 0,
    };

/* C test part 2 - "s7a" - loops and subprograms */

void f1 (int x);
void f2 (int x);
void f3 (int x);
void f4 (int x);
void f5 (int x);
void f6 (int x);
void f7 (int x);
void f8 (int x);
int f9 (int x);

void f1 (int x) { f2 (x); f3 (x); }
void f2 (int x) { f4 (x); }
void f3 (int x) { }
void f4 (int x) { f5 (x); }
void f5 (int x) { if (x < 10) { f5 (x + 1); } f6 (x); }
void f6 (int x) { 
   int i, j;
   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         f7 (i + j);
      }
      f8 (i);
   }
   f9 (x);
}
void f7 (int x) {
   int i;
   for (i = 0; i < 4; i++) {
      f8 (i);
   }
}

void f8 (int x) { }
int f9 (int x) {
   /* There are two ways into this loop. That is one way to make it
    * irreducible. The header is not well defined. */
   int i = 5;
   if (x) { goto start; }

   for (i = 0; i < 10; i++) {
      x++;
start:
      x++;
   }
   return x;
}

/* C test part 3 - zlib decompression */


void crash_longjmp (unsigned env, int v) {
   while (1) {}
}

/* puff.h
  Copyright (C) 2002-2010 Mark Adler, all rights reserved
  version 2.2, 25 Apr 2010

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
 */


/*
 * See puff.c for purpose and usage.
 */
#ifndef NIL
#  define NIL ((unsigned char *)0)      /* for no output option */
#endif

int puff(unsigned char *dest,           /* pointer to destination pointer */
         unsigned long *destlen,        /* amount of output space */
         const unsigned char *source,   /* pointer to source data pointer */
         unsigned long *sourcelen);     /* amount of input available */


/*
 * puff.c
 * Copyright (C) 2002-2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in puff.h
 * version 2.2, 25 Apr 2010
 *
 * puff.c is a simple inflate written to be an unambiguous way to specify the
 * deflate format.  It is not written for speed but rather simplicity.  As a
 * side benefit, this code might actually be useful when small code is more
 * important than speed, such as bootstrap applications.  For typical deflate
 * data, zlib's inflate() is about four times as fast as puff().  zlib's
 * inflate compiles to around 20K on my machine, whereas puff.c compiles to
 * around 4K on my machine (a PowerPC using GNU cc).  If the faster decode()
 * function here is used, then puff() is only twice as slow as zlib's
 * inflate().
 *
 * All dynamically allocated memory comes from the stack.  The stack required
 * is less than 2K bytes.  This code is compatible with 16-bit int's and
 * assumes that long's are at least 32 bits.  puff.c uses the short data type,
 * assumed to be 16 bits, for arrays in order to to conserve memory.  The code
 * works whether integers are stored big endian or little endian.
 *
 * In the comments below are "Format notes" that describe the inflate process
 * and document some of the less obvious aspects of the format.  This source
 * code is meant to supplement RFC 1951, which formally describes the deflate
 * format:
 *
 *    http://www.zlib.org/rfc-deflate.html
 */

/*
 * Change history:
 *
 * 1.0  10 Feb 2002     - First version
 * 1.1  17 Feb 2002     - Clarifications of some comments and notes
 *                      - Update puff() dest and source pointers on negative
 *                        errors to facilitate debugging deflators
 *                      - Remove longest from struct huffman -- not needed
 *                      - Simplify offs[] index in construct()
 *                      - Add input size and checking, using longjmp() to
 *                        maintain easy readability
 *                      - Use short data type for large arrays
 *                      - Use pointers instead of long to specify source and
 *                        destination sizes to avoid arbitrary 4 GB limits
 * 1.2  17 Mar 2002     - Add faster version of decode(), doubles speed (!),
 *                        but leave simple version for readabilty
 *                      - Make sure invalid distances detected if pointers
 *                        are 16 bits
 *                      - Fix fixed codes table error
 *                      - Provide a scanning mode for determining size of
 *                        uncompressed data
 * 1.3  20 Mar 2002     - Go back to lengths for puff() parameters [Gailly]
 *                      - Add a puff.h file for the interface
 *                      - Add braces in puff() for else do [Gailly]
 *                      - Use indexes instead of pointers for readability
 * 1.4  31 Mar 2002     - Simplify construct() code set check
 *                      - Fix some comments
 *                      - Add FIXLCODES #define
 * 1.5   6 Apr 2002     - Minor comment fixes
 * 1.6   7 Aug 2002     - Minor format changes
 * 1.7   3 Mar 2003     - Added test code for distribution
 *                      - Added zlib-like license
 * 1.8   9 Jan 2004     - Added some comments on no distance codes case
 * 1.9  21 Feb 2008     - Fix bug on 16-bit integer architectures [Pohland]
 *                      - Catch missing end-of-block symbol error
 * 2.0  25 Jul 2008     - Add #define to permit distance too far back
 *                      - Add option in TEST code for puff to write the data
 *                      - Add option in TEST code to skip input bytes
 *                      - Allow TEST code to read from piped stdin
 * 2.1   4 Apr 2010     - Avoid variable initialization for happier compilers
 *                      - Avoid unsigned comparisons for even happier compilers
 * 2.2  25 Apr 2010     - Fix bug in variable initializations [Oberhumer]
 *                      - Add const where appropriate [Oberhumer]
 *                      - Split if's and ?'s for coverage testing
 *                      - Break out test code to separate file
 *                      - Move NIL to puff.h
 *                      - Allow incomplete code only if single code length is 1
 *                      - Add full code coverage test to Makefile
 */


#define local static            /* for local function definitions */

/*
 * Maximums for allocations and loops.  It is not useful to change these --
 * they are fixed by the deflate format.
 */
#define MAXBITS 15              /* maximum bits in a code */
#define MAXLCODES 286           /* maximum number of literal/length codes */
#define MAXDCODES 30            /* maximum number of distance codes */
#define MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#define FIXLCODES 288           /* number of fixed literal/length codes */

/* input and output state */
struct state {
    /* output state */
    unsigned char *out;         /* output buffer */
    unsigned long outlen;       /* available space at out */
    unsigned long outcnt;       /* bytes written to out so far */

    /* input state */
    const unsigned char *in;    /* input buffer */
    unsigned long inlen;        /* available input at in */
    unsigned long incnt;        /* bytes read so far */
    int bitbuf;                 /* bit buffer */
    int bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    unsigned env;
};

/*
 * Return need bits from the input stream.  This always leaves less than
 * eight bits in the buffer.  bits() works properly for need == 0.
 *
 * Format notes:
 *
 * - Bits are stored in bytes from the least significant bit to the most
 *   significant bit.  Therefore bits are dropped from the bottom of the bit
 *   buffer, using shift right, and new bytes are appended to the top of the
 *   bit buffer, using shift left.
 */
local int bits(struct state *s, int need)
{
    long val;           /* bit accumulator (can use up to 20 bits) */

    /* load at least need bits into val */
    val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->incnt == s->inlen)
            crash_longjmp(s->env, 1);         /* out of input */
        val |= (long)(s->in[s->incnt++]) << s->bitcnt;  /* load eight bits */
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = (int)(val >> need);
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    return (int)(val & ((1L << need) - 1));
}

/*
 * Process a stored block.
 *
 * Format notes:
 *
 * - After the two-bit stored block type (00), the stored block length and
 *   stored bytes are byte-aligned for fast copying.  Therefore any leftover
 *   bits in the byte that has the last bit of the type, as many as seven, are
 *   discarded.  The value of the discarded bits are not defined and should not
 *   be checked against any expectation.
 *
 * - The second inverted copy of the stored block length does not have to be
 *   checked, but it's probably a good idea to do so anyway.
 *
 * - A stored block can have zero length.  This is sometimes used to byte-align
 *   subsets of the compressed data for random access or partial recovery.
 */
local int stored(struct state *s)
{
    unsigned len;       /* length of stored block */

    /* discard leftover bits from current byte (assumes s->bitcnt < 8) */
    s->bitbuf = 0;
    s->bitcnt = 0;

    /* get length and check against its one's complement */
    if (s->incnt + 4 > s->inlen)
        return 2;                               /* not enough input */
    len = s->in[s->incnt++];
    len |= s->in[s->incnt++] << 8;
    if (s->in[s->incnt++] != (~len & 0xff) ||
        s->in[s->incnt++] != ((~len >> 8) & 0xff))
        return -2;                              /* didn't match complement! */

    /* copy len bytes from in to out */
    if (s->incnt + len > s->inlen)
        return 2;                               /* not enough input */
    if (s->out != NIL) {
        if (s->outcnt + len > s->outlen)
            return 1;                           /* not enough output space */
        while (len--)
            s->out[s->outcnt++] = s->in[s->incnt++];
    }
    else {                                      /* just scanning */
        s->outcnt += len;
        s->incnt += len;
    }

    /* done with a valid stored block */
    return 0;
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
struct huffman {
    short *count;       /* number of symbols of each length */
    short *symbol;      /* canonically ordered symbols */
};

/*
 * Decode a code from the stream s using huffman table h.  Return the symbol or
 * a negative value if there is an error.  If all of the lengths are zero, i.e.
 * an empty code, or if the code is incomplete and an invalid code is received,
 * then -10 is returned after reading MAXBITS bits.
 *
 * Format notes:
 *
 * - The codes as stored in the compressed data are bit-reversed relative to
 *   a simple integer ordering of codes of the same lengths.  Hence below the
 *   bits are pulled from the compressed data one at a time and used to
 *   build the code value reversed from what is in the stream in order to
 *   permit simple integer comparisons for decoding.  A table-based decoding
 *   scheme (as used in zlib) does not need to do this reversal.
 *
 * - The first code for the shortest length is all zeros.  Subsequent codes of
 *   the same length are simply integer increments of the previous code.  When
 *   moving up a length, a zero bit is appended to the code.  For a complete
 *   code, the last code of the longest length will be all ones.
 *
 * - Incomplete codes are handled by this decoder, since they are permitted
 *   in the deflate format.  See the format notes for fixed() and dynamic().
 */
#ifdef SLOW
local int decode(struct state *s, const struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */

    code = first = index = 0;
    for (len = 1; len <= MAXBITS; len++) {
        code |= bits(s, 1);             /* get next bit */
        count = h->count[len];
        if (code - count < first)       /* if length len, return symbol */
            return h->symbol[index + (code - first)];
        index += count;                 /* else update for next length */
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -10;                         /* ran out of codes */
}

/*
 * A faster version of decode() for real applications of this code.   It's not
 * as readable, but it makes puff() twice as fast.  And it only makes the code
 * a few percent larger.
 */
#else /* !SLOW */
local int decode(struct state *s, const struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */
    int bitbuf;         /* bits from stream */
    int left;           /* bits left in next or left to process */
    short *next;        /* next number of codes */

    bitbuf = s->bitbuf;
    left = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->count + 1;
    while (1) {
        while (left--) {
            code |= bitbuf & 1;
            bitbuf >>= 1;
            count = *next++;
            if (code - count < first) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                return h->symbol[index + (code - first)];
            }
            index += count;             /* else update for next length */
            first += count;
            first <<= 1;
            code <<= 1;
            len++;
        }
        left = (MAXBITS+1) - len;
        if (left == 0)
            break;
        if (s->incnt == s->inlen)
            crash_longjmp(s->env, 1);         /* out of input */
        bitbuf = s->in[s->incnt++];
        if (left > 8)
            left = 8;
    }
    return -10;                         /* ran out of codes */
}
#endif /* SLOW */

/*
 * Given the list of code lengths length[0..n-1] representing a canonical
 * Huffman code for n symbols, construct the tables required to decode those
 * codes.  Those tables are the number of codes of each length, and the symbols
 * sorted by length, retaining their original order within each length.  The
 * return value is zero for a complete code set, negative for an over-
 * subscribed code set, and positive for an incomplete code set.  The tables
 * can be used if the return value is zero or positive, but they cannot be used
 * if the return value is negative.  If the return value is zero, it is not
 * possible for decode() using that table to return an error--any stream of
 * enough bits will resolve to a symbol.  If the return value is positive, then
 * it is possible for decode() using that table to return an error for received
 * codes past the end of the incomplete lengths.
 *
 * Not used by decode(), but used for error checking, h->count[0] is the number
 * of the n symbols not in the code.  So n - h->count[0] is the number of
 * codes.  This is useful for checking for incomplete codes that have more than
 * one symbol, which is an error in a dynamic block.
 *
 * Assumption: for all i in 0..n-1, 0 <= length[i] <= MAXBITS
 * This is assured by the construction of the length arrays in dynamic() and
 * fixed() and is not verified by construct().
 *
 * Format notes:
 *
 * - Permitted and expected examples of incomplete codes are one of the fixed
 *   codes and any code with a single symbol which in deflate is coded as one
 *   bit instead of zero bits.  See the format notes for fixed() and dynamic().
 *
 * - Within a given code length, the symbols are kept in ascending order for
 *   the code bits definition.
 */
local int construct(struct huffman *h, const short *length, int n)
{
    int symbol;         /* current symbol when stepping through length[] */
    int len;            /* current length when stepping through h->count[] */
    int left;           /* number of possible codes left of current length */
    short offs[MAXBITS+1];      /* offsets in symbol table for each length */

    /* count number of codes of each length */
    for (len = 0; len <= MAXBITS; len++)
        h->count[len] = 0;
    for (symbol = 0; symbol < n; symbol++)
        (h->count[length[symbol]])++;   /* assumes lengths are within bounds */
    if (h->count[0] == n)               /* no codes! */
        return 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;                           /* one possible code of zero length */
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;                     /* one more bit, double codes left */
        left -= h->count[len];          /* deduct count from possible codes */
        if (left < 0)
            return left;                /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    for (symbol = 0; symbol < n; symbol++)
        if (length[symbol] != 0)
            h->symbol[offs[length[symbol]]++] = symbol;

    /* return zero for complete set, positive for incomplete set */
    return left;
}

/*
 * Decode literal/length and distance codes until an end-of-block code.
 *
 * Format notes:
 *
 * - Compressed data that is after the block type if fixed or after the code
 *   description if dynamic is a combination of literals and length/distance
 *   pairs terminated by and end-of-block code.  Literals are simply Huffman
 *   coded bytes.  A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - Literals, lengths, and the end-of-block code are combined into a single
 *   code of up to 286 symbols.  They are 256 literals (0..255), 29 length
 *   symbols (257..285), and the end-of-block symbol (256).
 *
 * - There are 256 possible lengths (3..258), and so 29 symbols are not enough
 *   to represent all of those.  Lengths 3..10 and 258 are in fact represented
 *   by just a length symbol.  Lengths 11..257 are represented as a symbol and
 *   some number of extra bits that are added as an integer to the base length
 *   of the length symbol.  The number of extra bits is determined by the base
 *   length symbol.  These are in the static arrays below, lens[] for the base
 *   lengths and lext[] for the corresponding number of extra bits.
 *
 * - The reason that 258 gets its own symbol is that the longest length is used
 *   often in highly redundant files.  Note that 258 can also be coded as the
 *   base value 227 plus the maximum extra value of 31.  While a good deflate
 *   should never do this, it is not an error, and should be decoded properly.
 *
 * - If a length is decoded, including its extra bits if any, then it is
 *   followed a distance code.  There are up to 30 distance symbols.  Again
 *   there are many more possible distances (1..32768), so extra bits are added
 *   to a base value represented by the symbol.  The distances 1..4 get their
 *   own symbol, but the rest require extra bits.  The base distances and
 *   corresponding number of extra bits are below in the static arrays dist[]
 *   and dext[].
 *
 * - Literal bytes are simply written to the output.  A length/distance pair is
 *   an instruction to copy previously uncompressed bytes to the output.  The
 *   copy is from distance bytes back in the output stream, copying for length
 *   bytes.
 *
 * - Distances pointing before the beginning of the output data are not
 *   permitted.
 *
 * - Overlapped copies, where the length is greater than the distance, are
 *   allowed and common.  For example, a distance of one and a length of 258
 *   simply copies the last byte 258 times.  A distance of four and a length of
 *   twelve copies the last four bytes three times.  A simple forward copy
 *   ignoring whether the length is greater than the distance or not implements
 *   this correctly.  You should not use memcpy() since its behavior is not
 *   defined for overlapped arrays.  You should not use memmove() or bcopy()
 *   since though their behavior -is- defined for overlapping arrays, it is
 *   defined to do the wrong thing in this case.
 */
local int codes(struct state *s,
                const struct huffman *lencode,
                const struct huffman *distcode)
{
    int symbol;         /* decoded symbol */
    int len;            /* length for copy */
    unsigned dist;      /* distance for copy */
    static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    /* decode literals and length/distance pairs */
    do {
        symbol = decode(s, lencode);
        if (symbol < 0)
            return symbol;              /* invalid symbol */
        if (symbol < 256) {             /* literal: symbol is the byte */
            /* write out the literal */
            if (s->out != NIL) {
                if (s->outcnt == s->outlen)
                    return 1;
                s->out[s->outcnt] = symbol;
            }
            s->outcnt++;
        }
        else if (symbol > 256) {        /* length */
            /* get and compute length */
            symbol -= 257;
            if (symbol >= 29)
                return -10;             /* invalid fixed code */
            len = lens[symbol] + bits(s, lext[symbol]);

            /* get and check distance */
            symbol = decode(s, distcode);
            if (symbol < 0)
                return symbol;          /* invalid symbol */
            dist = dists[symbol] + bits(s, dext[symbol]);
#ifndef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
            if (dist > s->outcnt)
                return -11;     /* distance too far back */
#endif

            /* copy length bytes from distance bytes back */
            if (s->out != NIL) {
                if (s->outcnt + len > s->outlen)
                    return 1;
                while (len--) {
                    s->out[s->outcnt] =
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        dist > s->outcnt ?
                            0 :
#endif
                            s->out[s->outcnt - dist];
                    s->outcnt++;
                }
            }
            else
                s->outcnt += len;
        }
    } while (symbol != 256);            /* end of block symbol */

    /* done with a valid fixed or dynamic block */
    return 0;
}

/*
 * Process a fixed codes block.
 *
 * Format notes:
 *
 * - This block type can be useful for compressing small amounts of data for
 *   which the size of the code descriptions in a dynamic block exceeds the
 *   benefit of custom codes for that block.  For fixed codes, no bits are
 *   spent on code descriptions.  Instead the code lengths for literal/length
 *   codes and distance codes are fixed.  The specific lengths for each symbol
 *   can be seen in the "for" loops below.
 *
 * - The literal/length code is complete, but has two symbols that are invalid
 *   and should result in an error if received.  This cannot be implemented
 *   simply as an incomplete code since those two symbols are in the "middle"
 *   of the code.  They are eight bits long and the longest literal/length\
 *   code is nine bits.  Therefore the code must be constructed with those
 *   symbols, and the invalid symbols must be detected after decoding.
 *
 * - The fixed distance codes also have two invalid symbols that should result
 *   in an error if received.  Since all of the distance codes are the same
 *   length, this can be implemented as an incomplete code.  Then the invalid
 *   codes are detected while decoding.
 */
local int fixed(struct state *s)
{
    static int virgin = 1;
    static short lencnt[MAXBITS+1], lensym[FIXLCODES];
    static short distcnt[MAXBITS+1], distsym[MAXDCODES];
    static struct huffman lencode, distcode;

    /* build fixed huffman tables if first call (may not be thread safe) */
    if (virgin) {
        int symbol;
        short lengths[FIXLCODES];

        /* construct lencode and distcode */
        lencode.count = lencnt;
        lencode.symbol = lensym;
        distcode.count = distcnt;
        distcode.symbol = distsym;

        /* literal/length table */
        for (symbol = 0; symbol < 144; symbol++)
            lengths[symbol] = 8;
        for (; symbol < 256; symbol++)
            lengths[symbol] = 9;
        for (; symbol < 280; symbol++)
            lengths[symbol] = 7;
        for (; symbol < FIXLCODES; symbol++)
            lengths[symbol] = 8;
        construct(&lencode, lengths, FIXLCODES);

        /* distance table */
        for (symbol = 0; symbol < MAXDCODES; symbol++)
            lengths[symbol] = 5;
        construct(&distcode, lengths, MAXDCODES);

        /* do this just once */
        virgin = 0;
    }

    /* decode data until end-of-block code */
    return codes(s, &lencode, &distcode);
}

/*
 * Process a dynamic codes block.
 *
 * Format notes:
 *
 * - A dynamic block starts with a description of the literal/length and
 *   distance codes for that block.  New dynamic blocks allow the compressor to
 *   rapidly adapt to changing data with new codes optimized for that data.
 *
 * - The codes used by the deflate format are "canonical", which means that
 *   the actual bits of the codes are generated in an unambiguous way simply
 *   from the number of bits in each code.  Therefore the code descriptions
 *   are simply a list of code lengths for each symbol.
 *
 * - The code lengths are stored in order for the symbols, so lengths are
 *   provided for each of the literal/length symbols, and for each of the
 *   distance symbols.
 *
 * - If a symbol is not used in the block, this is represented by a zero as
 *   as the code length.  This does not mean a zero-length code, but rather
 *   that no code should be created for this symbol.  There is no way in the
 *   deflate format to represent a zero-length code.
 *
 * - The maximum number of bits in a code is 15, so the possible lengths for
 *   any code are 1..15.
 *
 * - The fact that a length of zero is not permitted for a code has an
 *   interesting consequence.  Normally if only one symbol is used for a given
 *   code, then in fact that code could be represented with zero bits.  However
 *   in deflate, that code has to be at least one bit.  So for example, if
 *   only a single distance base symbol appears in a block, then it will be
 *   represented by a single code of length one, in particular one 0 bit.  This
 *   is an incomplete code, since if a 1 bit is received, it has no meaning,
 *   and should result in an error.  So incomplete distance codes of one symbol
 *   should be permitted, and the receipt of invalid codes should be handled.
 *
 * - It is also possible to have a single literal/length code, but that code
 *   must be the end-of-block code, since every dynamic block has one.  This
 *   is not the most efficient way to create an empty block (an empty fixed
 *   block is fewer bits), but it is allowed by the format.  So incomplete
 *   literal/length codes of one symbol should also be permitted.
 *
 * - If there are only literal codes and no lengths, then there are no distance
 *   codes.  This is represented by one distance code with zero bits.
 *
 * - The list of up to 286 length/literal lengths and up to 30 distance lengths
 *   are themselves compressed using Huffman codes and run-length encoding.  In
 *   the list of code lengths, a 0 symbol means no code, a 1..15 symbol means
 *   that length, and the symbols 16, 17, and 18 are run-length instructions.
 *   Each of 16, 17, and 18 are follwed by extra bits to define the length of
 *   the run.  16 copies the last length 3 to 6 times.  17 represents 3 to 10
 *   zero lengths, and 18 represents 11 to 138 zero lengths.  Unused symbols
 *   are common, hence the special coding for zero lengths.
 *
 * - The symbols for 0..18 are Huffman coded, and so that code must be
 *   described first.  This is simply a sequence of up to 19 three-bit values
 *   representing no code (0) or the code length for that symbol (1..7).
 *
 * - A dynamic block starts with three fixed-size counts from which is computed
 *   the number of literal/length code lengths, the number of distance code
 *   lengths, and the number of code length code lengths (ok, you come up with
 *   a better name!) in the code descriptions.  For the literal/length and
 *   distance codes, lengths after those provided are considered zero, i.e. no
 *   code.  The code length code lengths are received in a permuted order (see
 *   the order[] array below) to make a short code length code length list more
 *   likely.  As it turns out, very short and very long codes are less likely
 *   to be seen in a dynamic code description, hence what may appear initially
 *   to be a peculiar ordering.
 *
 * - Given the number of literal/length code lengths (nlen) and distance code
 *   lengths (ndist), then they are treated as one long list of nlen + ndist
 *   code lengths.  Therefore run-length coding can and often does cross the
 *   boundary between the two sets of lengths.
 *
 * - So to summarize, the code description at the start of a dynamic block is
 *   three counts for the number of code lengths for the literal/length codes,
 *   the distance codes, and the code length codes.  This is followed by the
 *   code length code lengths, three bits each.  This is used to construct the
 *   code length code which is used to read the remainder of the lengths.  Then
 *   the literal/length code lengths and distance lengths are read as a single
 *   set of lengths using the code length codes.  Codes are constructed from
 *   the resulting two sets of lengths, and then finally you can start
 *   decoding actual compressed data in the block.
 *
 * - For reference, a "typical" size for the code description in a dynamic
 *   block is around 80 bytes.
 */
local int dynamic(struct state *s)
{
    int nlen, ndist, ncode;             /* number of lengths in descriptor */
    int index;                          /* index of lengths[] */
    int err;                            /* construct() return value */
    short lengths[MAXCODES];            /* descriptor code lengths */
    short lencnt[MAXBITS+1], lensym[MAXLCODES];         /* lencode memory */
    short distcnt[MAXBITS+1], distsym[MAXDCODES];       /* distcode memory */
    struct huffman lencode, distcode;   /* length and distance codes */
    static const short order[19] =      /* permutation of code length codes */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* construct lencode and distcode */
    lencode.count = lencnt;
    lencode.symbol = lensym;
    distcode.count = distcnt;
    distcode.symbol = distsym;

    /* get number of lengths in each table, check lengths */
    nlen = bits(s, 5) + 257;
    ndist = bits(s, 5) + 1;
    ncode = bits(s, 4) + 4;
    if (nlen > MAXLCODES || ndist > MAXDCODES)
        return -3;                      /* bad counts */

    /* read code length code lengths (really), missing lengths are zero */
    for (index = 0; index < ncode; index++)
        lengths[order[index]] = bits(s, 3);
    for (; index < 19; index++)
        lengths[order[index]] = 0;

    /* build huffman table for code lengths codes (use lencode temporarily) */
    err = construct(&lencode, lengths, 19);
    if (err != 0)               /* require complete code set here */
        return -4;

    /* read length/literal and distance code length tables */
    index = 0;
    while (index < nlen + ndist) {
        int symbol;             /* decoded value */
        int len;                /* last length to repeat */

        symbol = decode(s, &lencode);
        if (symbol < 16)                /* length in 0..15 */
            lengths[index++] = symbol;
        else {                          /* repeat instruction */
            len = 0;                    /* assume repeating zeros */
            if (symbol == 16) {         /* repeat last length 3..6 times */
                if (index == 0)
                    return -5;          /* no last length! */
                len = lengths[index - 1];       /* last length */
                symbol = 3 + bits(s, 2);
            }
            else if (symbol == 17)      /* repeat zero 3..10 times */
                symbol = 3 + bits(s, 3);
            else                        /* == 18, repeat zero 11..138 times */
                symbol = 11 + bits(s, 7);
            if (index + symbol > nlen + ndist)
                return -6;              /* too many lengths! */
            while (symbol--)            /* repeat last or zero symbol times */
                lengths[index++] = len;
        }
    }

    /* check for end-of-block code -- there better be one! */
    if (lengths[256] == 0)
        return -9;

    /* build huffman table for literal/length codes */
    err = construct(&lencode, lengths, nlen);
    if (err && (err < 0 || nlen != lencode.count[0] + lencode.count[1]))
        return -7;      /* incomplete code ok only for single length 1 code */

    /* build huffman table for distance codes */
    err = construct(&distcode, lengths + nlen, ndist);
    if (err && (err < 0 || ndist != distcode.count[0] + distcode.count[1]))
        return -8;      /* incomplete code ok only for single length 1 code */

    /* decode data until end-of-block code */
    return codes(s, &lencode, &distcode);
}

/*
 * Inflate source to dest.  On return, destlen and sourcelen are updated to the
 * size of the uncompressed data and the size of the deflate data respectively.
 * On success, the return value of puff() is zero.  If there is an error in the
 * source data, i.e. it is not in the deflate format, then a negative value is
 * returned.  If there is not enough input available or there is not enough
 * output space, then a positive error is returned.  In that case, destlen and
 * sourcelen are not updated to facilitate retrying from the beginning with the
 * provision of more input data or more output space.  In the case of invalid
 * inflate data (a negative error), the dest and source pointers are updated to
 * facilitate the debugging of deflators.
 *
 * puff() also has a mode to determine the size of the uncompressed output with
 * no output written.  For this dest must be (unsigned char *)0.  In this case,
 * the input value of *destlen is ignored, and on return *destlen is set to the
 * size of the uncompressed output.
 *
 * The return codes are:
 *
 *   2:  available inflate data did not terminate
 *   1:  output space exhausted before completing inflate
 *   0:  successful inflate
 *  -1:  invalid block type (type == 3)
 *  -2:  stored block length did not match one's complement
 *  -3:  dynamic block code description: too many length or distance codes
 *  -4:  dynamic block code description: code lengths codes incomplete
 *  -5:  dynamic block code description: repeat lengths with no first length
 *  -6:  dynamic block code description: repeat more than specified lengths
 *  -7:  dynamic block code description: invalid literal/length code lengths
 *  -8:  dynamic block code description: invalid distance code lengths
 *  -9:  dynamic block code description: missing end-of-block code
 * -10:  invalid literal/length or distance code in fixed or dynamic block
 * -11:  distance is too far back in fixed or dynamic block
 *
 * Format notes:
 *
 * - Three bits are read for each block to determine the kind of block and
 *   whether or not it is the last block.  Then the block is decoded and the
 *   process repeated if it was not the last block.
 *
 * - The leftover bits in the last byte of the deflate data after the last
 *   block (if it was a fixed or dynamic block) are undefined and have no
 *   expected values to check.
 */
int puff(unsigned char *dest,           /* pointer to destination pointer */
         unsigned long *destlen,        /* amount of output space */
         const unsigned char *source,   /* pointer to source data pointer */
         unsigned long *sourcelen)      /* amount of input available */
{
    struct state s;             /* input/output state */
    int last, type;             /* block information */
    int err;                    /* return value */

    /* initialize output state */
    s.out = dest;
    s.outlen = *destlen;                /* ignored if dest is NIL */
    s.outcnt = 0;

    /* initialize input state */
    s.in = source;
    s.inlen = *sourcelen;
    s.incnt = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* return if bits() or decode() tries to read past available input */
    {
        /* process blocks until last block or error */
        do {
            last = bits(&s, 1);         /* one if last block */
            type = bits(&s, 2);         /* block type 0..3 */
            err = type == 0 ?
                    stored(&s) :
                    (type == 1 ?
                        fixed(&s) :
                        (type == 2 ?
                            dynamic(&s) :
                            -1));       /* type == 3, invalid */
            if (err != 0)
                break;                  /* return with error */
        } while (!last);
    }

    /* update the lengths and return */
    if (err <= 0) {
        *destlen = s.outcnt;
        *sourcelen = s.incnt;
    }
    return err;
}

char output_data[700];
const char input_data [] = {
/*0x78,0x9c,*/0x55,0x52,0xcb,0xaa,0xdb,0x30,0x10,0xdd,0xeb,0x2b,0x66,0xd7,0x8d,
0xc9,0x0f,0x74,0x15,0x42,0xa1,0x59,0x5c,0x0a,0xb7,0x17,0x42,0x97,0x13,0x6b,
0x6c,0xab,0x91,0x67,0x82,0x1e,0x31,0xfe,0xfb,0x1e,0xc9,0xbd,0xd0,0x6e,0x24,
0x0d,0x9a,0xa3,0xf3,0x18,0xdd,0x16,0xa3,0x91,0x95,0x8a,0xc4,0x48,0xab,0x50,
0x98,0x68,0x13,0x5a,0xf8,0x85,0x45,0xb0,0xea,0xe0,0x6e,0x7f,0x5b,0x32,0xef,
0x54,0x16,0xa1,0x0d,0x7b,0x28,0x94,0x17,0xab,0xd1,0xd3,0x5d,0xbe,0xba,0x37,
0x33,0x8d,0x61,0x5e,0x0a,0x2d,0x16,0xe3,0x3e,0xf4,0xb6,0x9f,0xfc,0x7c,0x02,
0x79,0xb1,0x55,0xca,0xe0,0xce,0x3a,0x4b,0xfc,0x92,0xc1,0xc3,0x29,0x03,0x14,
0x6d,0x23,0xa6,0x92,0x44,0x4e,0xee,0x97,0x55,0x2a,0x1c,0x1f,0x64,0x53,0x47,
0xde,0x93,0x70,0x2f,0x56,0x4b,0x1a,0x74,0x76,0xe7,0x4c,0x3b,0x7a,0x5e,0x41,
0xb6,0xde,0xa0,0xd8,0xb9,0x26,0x4b,0x3c,0xb8,0x4b,0xb4,0xea,0x29,0x28,0x8d,
0x29,0xac,0xd9,0xf4,0x20,0x7f,0xc8,0xde,0x1e,0xf8,0xb4,0xf0,0x43,0x85,0xa2,
0xc1,0xd3,0xc8,0xe9,0x25,0xbd,0x9d,0x47,0xfe,0x6d,0xf5,0xd4,0xaf,0x8a,0xc1,
0x08,0xcc,0x03,0x71,0x59,0x82,0x32,0xbd,0x5b,0x96,0x7c,0xc0,0x20,0x12,0x46,
0x33,0xab,0x27,0x6d,0x16,0x33,0xb5,0xa3,0xe1,0xa6,0x97,0x83,0xfb,0xc6,0xa9,
0x2c,0x70,0x16,0x39,0x17,0x7a,0x86,0xb1,0xd4,0x24,0x87,0x08,0x69,0x8d,0x13,
0x09,0x24,0x34,0x17,0xdf,0x6b,0x67,0x08,0xea,0xc3,0x6c,0xfd,0x95,0x7b,0xac,
0xb0,0x7f,0xee,0x7e,0x56,0x64,0x48,0x51,0xd8,0xe7,0xa6,0xa4,0x98,0xbb,0x99,
0xe1,0x0c,0x80,0x47,0x1c,0xeb,0x41,0x7b,0xa5,0x09,0x01,0xdb,0xf6,0x09,0xda,
0x2c,0x41,0xf9,0xc6,0x01,0xb2,0x26,0x4b,0x40,0x62,0x18,0xfb,0x81,0x18,0x68,
0xdd,0xdb,0xac,0x4e,0xee,0x4a,0x0f,0xb5,0x96,0x1c,0x97,0x36,0xdf,0xeb,0xbf,
0xe3,0x75,0x1f,0x8b,0x24,0x8c,0x3d,0x93,0x5a,0x81,0xf7,0x19,0xd4,0xe4,0x25,
0x87,0x04,0x65,0xef,0xdc,0x82,0x02,0x6f,0x0a,0x2f,0x49,0x03,0xb2,0x38,0x08,
0x21,0x6a,0x33,0xf5,0x92,0xdc,0x1b,0xfe,0xc2,0x5d,0xe8,0xc9,0x89,0x7d,0xc8,
0x4d,0x37,0x34,0x34,0xc6,0x2c,0xd2,0x33,0xc8,0x55,0xff,0x2b,0x0b,0xc6,0x7f,
0x72,0x7f,0x00,0xf3,/*0x1d,0xd7,0x21,0x00*/};
unsigned long input_len = 382 - 6;
unsigned long output_len = 700;


/* C test part 4 - AES encryption */

void my_memcpy (void * dest, const void * src, unsigned size)
{
   unsigned i;
   for (i = 0; i < size; i++) {
      ((char *) dest)[i] = ((char *) src)[i];
   }
}

#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

#ifndef CTR
  #define CTR 1
#endif


#define AES128 1
//#define AES192 1
//#define AES256 1

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN 32
    #define AES_keyExpSize 240
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN 24
    #define AES_keyExpSize 208
#else
    #define AES_KEYLEN 16   // Key length in bytes
    #define AES_keyExpSize 176
#endif

struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
  uint8_t Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(ECB) && (ECB == 1)
// buffer size is exactly AES_BLOCKLEN bytes; 
// you need only AES_init_ctx as IV is not used in ECB 
// NB: ECB is considered insecure for most uses
void AES_ECB_encrypt(struct AES_ctx* ctx, uint8_t* buf);
void AES_ECB_decrypt(struct AES_ctx* ctx, uint8_t* buf);

#endif // #if defined(ECB) && (ECB == !)


#if defined(CBC) && (CBC == 1)
// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx via AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CBC_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CBC) && (CBC == 1)


#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting. 
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CTR) && (CTR == 1)


#endif //_AES_H_


/*

This is an implementation of the AES algorithm, specifically ECB, CTR and CBC mode.
Block size can be chosen in aes.h - available choices are AES128, AES192, AES256.

The implementation is verified against the test vectors in:
  National Institute of Standards and Technology Special Publication 800-38A 2001 ED

ECB-AES128
----------

  plain-text:
    6bc1bee22e409f96e93d7e117393172a
    ae2d8a571e03ac9c9eb76fac45af8e51
    30c81c46a35ce411e5fbc1191a0a52ef
    f69f2445df4f9b17ad2b417be66c3710

  key:
    2b7e151628aed2a6abf7158809cf4f3c

  resulting cipher
    3ad77bb40d7a3660a89ecaf32466ef97 
    f5d3d58503b9699de785895a96fdbaaf 
    43b1cd7f598ece23881b00e3ed030688 
    7b0c785e27e8ad3f8223207104725dd4 


NOTE:   String length must be evenly divisible by 16byte (str_len % 16 == 0)
        You should pad the end of the string with zeros if this is not the case.
        For AES192/256 the key size is proportionally larger.

*/


/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
// #include <stdint.h>
// #include <string.h> // CBC mode, for memset

/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/
// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

#if defined(AES256) && (AES256 == 1)
    #define Nk 8
    #define Nr 14
#elif defined(AES192) && (AES192 == 1)
    #define Nk 6
    #define Nr 12
#else
    #define Nk 4        // The number of 32 bit words in a key.
    #define Nr 10       // The number of rounds in AES Cipher.
#endif

// jcallan@github points out that declaring Multiply as a function 
// reduces code size considerably with the Keil ARM compiler.
// See this link for more information: https://github.com/kokke/tiny-AES-C/pull/3
#ifndef MULTIPLY_AS_A_FUNCTION
  #define MULTIPLY_AS_A_FUNCTION 0
#endif




/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/
// state - array holding the intermediate results during decryption.
typedef uint8_t state_t[4][4];



// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
// The numbers below can be computed dynamically trading ROM for RAM - 
// This can be useful in (embedded) bootloader applications, where ROM is often limited.
static const uint8_t sbox[256] = {
  //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t rsbox[256] = {
  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

// The round constant word array, Rcon[i], contains the values given by 
// x to the power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
static const uint8_t Rcon[11] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

/*
 * Jordan Goulder points out in PR #12 (https://github.com/kokke/tiny-AES-C/pull/12),
 * that you can remove most of the elements in the Rcon array, because they are unused.
 *
 * From Wikipedia's article on the Rijndael key schedule @ https://en.wikipedia.org/wiki/Rijndael_key_schedule#Rcon
 * 
 * "Only the first some of these constants are actually used – up to rcon[10] for AES-128 (as 11 round keys are needed), 
 *  up to rcon[8] for AES-192, up to rcon[7] for AES-256. rcon[0] is not used in AES algorithm."
 */


/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/
/*
static uint8_t getSBoxValue(uint8_t num)
{
  return sbox[num];
}
*/
#define getSBoxValue(num) (sbox[(num)])
/*
static uint8_t getSBoxInvert(uint8_t num)
{
  return rsbox[num];
}
*/
#define getSBoxInvert(num) (rsbox[(num)])

// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states. 
static void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key)
{
  unsigned i, j, k;
  uint8_t tempa[4]; // Used for the column/row operations
  
  // The first round key is the key itself.
  for (i = 0; i < Nk; ++i)
  {
    RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
    RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
    RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
    RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
  }

  // All other round keys are found from the previous round keys.
  for (i = Nk; i < Nb * (Nr + 1); ++i)
  {
    {
      k = (i - 1) * 4;
      tempa[0]=RoundKey[k + 0];
      tempa[1]=RoundKey[k + 1];
      tempa[2]=RoundKey[k + 2];
      tempa[3]=RoundKey[k + 3];

    }

    if (i % Nk == 0)
    {
      // This function shifts the 4 bytes in a word to the left once.
      // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

      // Function RotWord()
      {
        k = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = k;
      }

      // SubWord() is a function that takes a four-byte input word and 
      // applies the S-box to each of the four bytes to produce an output word.

      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }

      tempa[0] = tempa[0] ^ Rcon[i/Nk];
    }
#if defined(AES256) && (AES256 == 1)
    if (i % Nk == 4)
    {
      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }
    }
#endif
    j = i * 4; k=(i - Nk) * 4;
    RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
    RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
    RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
    RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
  }
}

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key)
{
  KeyExpansion(ctx->RoundKey, key);
}
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv)
{
  KeyExpansion(ctx->RoundKey, key);
  my_memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv)
{
  my_memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}
#endif

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
static void AddRoundKey(uint8_t round,state_t* state,uint8_t* RoundKey)
{
  uint8_t i,j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
    }
  }
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void SubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxValue((*state)[j][i]);
    }
  }
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
static void ShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to left  
  temp           = (*state)[0][1];
  (*state)[0][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[3][1];
  (*state)[3][1] = temp;

  // Rotate second row 2 columns to left  
  temp           = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp           = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to left
  temp           = (*state)[0][3];
  (*state)[0][3] = (*state)[3][3];
  (*state)[3][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[1][3];
  (*state)[1][3] = temp;
}

static uint8_t xtime(uint8_t x)
{
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix
static void MixColumns(state_t* state)
{
  uint8_t i;
  uint8_t Tmp, Tm, t;
  for (i = 0; i < 4; ++i)
  {  
    t   = (*state)[i][0];
    Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
    Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
  }
}

// Multiply is used to multiply numbers in the field GF(2^8)
// Note: The last call to xtime() is unneeded, but often ends up generating a smaller binary
//       The compiler seems to be able to vectorize the operation better this way.
//       See https://github.com/kokke/tiny-AES-c/pull/34
#if MULTIPLY_AS_A_FUNCTION
static uint8_t Multiply(uint8_t x, uint8_t y)
{
  return (((y & 1) * x) ^
       ((y>>1 & 1) * xtime(x)) ^
       ((y>>2 & 1) * xtime(xtime(x))) ^
       ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^
       ((y>>4 & 1) * xtime(xtime(xtime(xtime(x)))))); /* this last call to xtime() can be omitted */
  }
#else
#define Multiply(x, y)                                \
      (  ((y & 1) * x) ^                              \
      ((y>>1 & 1) * xtime(x)) ^                       \
      ((y>>2 & 1) * xtime(xtime(x))) ^                \
      ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^         \
      ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))   \

#endif

// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
// Please use the references to gain more information.
static void InvMixColumns(state_t* state)
{
  int i;
  uint8_t a, b, c, d;
  for (i = 0; i < 4; ++i)
  { 
    a = (*state)[i][0];
    b = (*state)[i][1];
    c = (*state)[i][2];
    d = (*state)[i][3];

    (*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
    (*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
    (*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
    (*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
  }
}


// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void InvSubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxInvert((*state)[j][i]);
    }
  }
}

static void InvShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to right  
  temp = (*state)[3][1];
  (*state)[3][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[0][1];
  (*state)[0][1] = temp;

  // Rotate second row 2 columns to right 
  temp = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to right
  temp = (*state)[0][3];
  (*state)[0][3] = (*state)[1][3];
  (*state)[1][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[3][3];
  (*state)[3][3] = temp;
}


// Cipher is the main function that encrypts the PlainText.
static void Cipher(state_t* state, uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(0, state, RoundKey); 
  
  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr-1 rounds are executed in the loop below.
  for (round = 1; round < Nr; ++round)
  {
    SubBytes(state);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(round, state, RoundKey);
  }
  
  // The last round is given below.
  // The MixColumns function is not here in the last round.
  SubBytes(state);
  ShiftRows(state);
  AddRoundKey(Nr, state, RoundKey);
}

static void InvCipher(state_t* state,uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(Nr, state, RoundKey); 

  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr-1 rounds are executed in the loop below.
  for (round = (Nr - 1); round > 0; --round)
  {
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(round, state, RoundKey);
    InvMixColumns(state);
  }
  
  // The last round is given below.
  // The MixColumns function is not here in the last round.
  InvShiftRows(state);
  InvSubBytes(state);
  AddRoundKey(0, state, RoundKey);
}


/*****************************************************************************/
/* Public functions:                                                         */
/*****************************************************************************/
#if defined(ECB) && (ECB == 1)


void AES_ECB_encrypt(struct AES_ctx *ctx, uint8_t* buf)
{
  // The next function call encrypts the PlainText with the Key using AES algorithm.
  Cipher((state_t*)buf, ctx->RoundKey);
}

void AES_ECB_decrypt(struct AES_ctx* ctx, uint8_t* buf)
{
  // The next function call decrypts the PlainText with the Key using AES algorithm.
  InvCipher((state_t*)buf, ctx->RoundKey);
}


#endif // #if defined(ECB) && (ECB == 1)





#if defined(CBC) && (CBC == 1)


static void XorWithIv(uint8_t* buf, uint8_t* Iv)
{
  uint8_t i;
  for (i = 0; i < AES_BLOCKLEN; ++i) // The block in AES is always 128bit no matter the key size
  {
    buf[i] ^= Iv[i];
  }
}

void AES_CBC_encrypt_buffer(struct AES_ctx *ctx,uint8_t* buf, uint32_t length)
{
  uintptr_t i;
  uint8_t *Iv = ctx->Iv;
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    XorWithIv(buf, Iv);
    Cipher((state_t*)buf, ctx->RoundKey);
    Iv = buf;
    buf += AES_BLOCKLEN;
    //printf("Step %d - %d", i/16, i);
  }
  /* store Iv in ctx for next call */
  my_memcpy(ctx->Iv, Iv, AES_BLOCKLEN);
}

void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf,  uint32_t length)
{
  uintptr_t i;
  uint8_t storeNextIv[AES_BLOCKLEN];
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    my_memcpy(storeNextIv, buf, AES_BLOCKLEN);
    InvCipher((state_t*)buf, ctx->RoundKey);
    XorWithIv(buf, ctx->Iv);
    my_memcpy(ctx->Iv, storeNextIv, AES_BLOCKLEN);
    buf += AES_BLOCKLEN;
  }

}

#endif // #if defined(CBC) && (CBC == 1)



#if defined(CTR) && (CTR == 1)

/* Symmetrical operation: same function for encrypting as for decrypting. Note any IV/nonce should never be reused with the same key */
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length)
{
  uint8_t buffer[AES_BLOCKLEN];
  
  unsigned i;
  int bi;
  for (i = 0, bi = AES_BLOCKLEN; i < length; ++i, ++bi)
  {
    if (bi == AES_BLOCKLEN) /* we need to regen xor compliment in buffer */
    {
      
      my_memcpy(buffer, ctx->Iv, AES_BLOCKLEN);
      Cipher((state_t*)buffer,ctx->RoundKey);

      /* Increment Iv and handle overflow */
      for (bi = (AES_BLOCKLEN - 1); bi >= 0; --bi)
      {
	/* inc will owerflow */
        if (ctx->Iv[bi] == 255)
	{
          ctx->Iv[bi] = 0;
          continue;
        } 
        ctx->Iv[bi] += 1;
        break;   
      }
      bi = 0;
    }

    buf[i] = (buf[i] ^ buffer[bi]);
  }
}

#endif // #if defined(CTR) && (CTR == 1)



int test_6(void)
{
   int i;
   struct AES_ctx ctx;
   unsigned char enc_data[32];

   /* C test part 1 - "arm2" - libgcc functions */
   for (i = 0; target_list[i]; i++) {
      target_list[i] ();
   }

   /* C test part 2 - "s7a" - loops and subprograms */
   f1 (0);
   f6 (0);

   /* C test part 3 - zlib decompression */
   i = puff ((uint8_t *) output_data, &output_len, (const uint8_t *) input_data, &input_len);
   if ((i != 0) || (output_len != 630) || (output_data[0x5a] != 'S') || (output_data[0x272] != 'r')) {
      /* decompress failed */
      while (1) {}
   }

   /* C test part 4 - AES encryption */
   my_memcpy (enc_data, output_data, 32);
   AES_init_ctx(&ctx, (const uint8_t *) input_data);
   AES_ECB_encrypt(&ctx, enc_data);
   if ((enc_data[1] != 0x94) || (enc_data[7] != 0x3a)) {
      /* encryption failed */
      while (1) {}
   }
   AES_ECB_decrypt(&ctx, enc_data);
   for (i = 0; i < 32; i++) {
      if (enc_data[i] != output_data[i]) {
         /* decryption failed */
         while (1) {}
      }
   }
   return 0;
}

