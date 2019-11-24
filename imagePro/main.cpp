//
//  main.cpp
//  imagePro
//
//  Created by Choo Xuan Wing on 15/11/2019.
//  Copyright © 2019 Choo Xuan Wing. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include "puff.h"
#include <vector>
#include <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#include "puff.h"               /* prototype for puff() */

// deflate puff code start *************************************************
// below also includes my own class file and CRC check from w3.com/TR/PNG
// Copyright (C) 2002-2013 Mark Adler

#define local static            /* for local function definitions */
#define MAXBITS 15              /* maximum bits in a code */
#define MAXLCODES 286           /* maximum number of literal/length codes */
#define MAXDCODES 30            /* maximum number of distance codes */
#define MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#define FIXLCODES 288           /* number of fixed literal/length codes */

using namespace std;

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
    jmp_buf env;
};

struct huffman {
    short *count;       /* number of symbols of each length */
    short *symbol;      /* canonically ordered symbols */
};


class file{			// initiate class to store sections of png
public:
//	uint32_t test,rawCode;
	vector <unsigned char> rawCode, IDATdata;
//	vector <unsigned int> rawDec;
	string fileName;
	vector<int> IDATloc,IDATlen;
	unsigned long int IHDRloc,IHDRlen, IENDloc, IENDlen, PHYSloc, PHYSlen, CHRMloc, CHRMlen, Tlen, crcIEND,crcIHDR,crcPHYS,crcCHRM;
	int width, height, bitdepth,colortype, compMethod,filtMethod,intlMethod;
	
};


class file file;


local int bits(struct state *s, int need)
{
    long val;           /* bit accumulator (can use up to 20 bits) */

    /* load at least need bits into val */
    val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->incnt == s->inlen)
            longjmp(s->env, 1);         /* out of input */
        val |= (long)(s->in[s->incnt++]) << s->bitcnt;  /* load eight bits */
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = (int)(val >> need);
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    return (int)(val & ((1L << need) - 1));
}


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
            longjmp(s->env, 1);         /* out of input */
        bitbuf = s->in[s->incnt++];
        if (left > 8)
            left = 8;
    }
    return -10;                         /* ran out of codes */
}
#endif /* SLOW */

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
        if (symbol < 0)
            return symbol;          /* invalid symbol */
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
    if (setjmp(s.env) != 0)             /* if came back here via longjmp() */
        err = 2;                        /* then skip do-loop, return error */
    else {
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

// CRC chrck *****************************************

/* Table of CRCs of all 8-bit messages. */
 unsigned long crc_table[256];
 
 /* Flag: has the table been computed? Initially false. */
 int crc_table_computed = 0;
 
 /* Make the table for a fast CRC. */
 void make_crc_table(void)
 {
   unsigned long c;
   int n, k;
 
   for (n = 0; n < 256; n++) {
	 c = (unsigned long) n;
	 for (k = 0; k < 8; k++) {
	   if (c & 1)
		 c = 0xedb88320L ^ (c >> 1);
	   else
		 c = c >> 1;
	 }
	 crc_table[n] = c;
   }
   crc_table_computed = 1;
 }

 /* Update a running CRC with the bytes buf[0..len-1]--the CRC
	should be initialized to all 1's, and the transmitted value
	is the 1's complement of the final running CRC (see the
	crc() routine below). */
 
 unsigned long update_crc(unsigned long crc, unsigned char *buf,
						  int len)
 {
   unsigned long c = crc;
   int n;
 
   if (!crc_table_computed)
	 make_crc_table();
   for (n = 0; n < len; n++) {
	 c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
   }
   return c;
 }
 
 /* Return the CRC of the bytes buf[0..len-1]. */
 unsigned long crc(unsigned char *buf, int len)
 {
   return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
 }


// CRC check end ***************************************

void setInitial(){		// sets initial values for var in class
	
	file.Tlen=0;
	file.IHDRlen=0;
	file.width=0;
	file.height=0;
	file.bitdepth=0;
	file.colortype=0;
	file.compMethod=0;
	file.filtMethod=0;
	file.intlMethod=0;
	file.IHDRloc=0;
	
}

void openSortFile(string fileName){

	string signature,temp;

	ifstream fopen(fileName,std::ios::binary);		// rb is read in binary(hex)
	
	if (fopen.fail()){
		cout << "File not found"<< endl;
		exit(1);							// Quits programme if file not found
	}
	
	else{
		cout << "Loading " << fileName<<endl;
		ostringstream ss;						// Use string stream instead of
		ss<<fopen.rdbuf();						// getline because getline ignores
		signature=ss.str();						// whitespace and stuff like 0x1a
		stringstream line(signature);
			
		//	make ss a buffer, rdbuf reads fopen char by char and puts into ss.
		//	.str func makes ss a string and keeps it in signature
		//	stringstream primes signature so it can be streamed for processing as a string
			 
		for (int i=0;i<signature.length();i++){		// put stringstremed values into class
			file.rawCode.push_back(signature.at(i));
//			file.rawDec.push_back(signature.at(i));
//			signature.at(i) >> std::ios::dec >> file.rawDec.at(i);
		}
		
		if(file.rawCode.at(0)==137 && file.rawCode.at(1)==80 &&
			file.rawCode.at(2)==78 && file.rawCode.at(3)==71 &&
			file.rawCode.at(4)==13 && file.rawCode.at(5)==10 &&
			file.rawCode.at(6)==26 && file.rawCode.at(7)==10){
			
			// must have signature of file in
			// decimal, 137 80 78 71 13 10 26 10 for all PNG
			
			cout << "\nSignature Valid"<< endl;
		}

		else{
			cout << "Signature invalid for PNG" << endl;
			exit(1);
		}
	}
}

void findIHDR(){		// ____ICHK____
						//		  ^ up pointer is val of i
	
	for (int i=0; i<file.rawCode.size();i++){
		// Find IHDR chunk location with ascii
		if (file.rawCode.at(i)==0x49 && file.rawCode.at(i+1)==0x48 && file.rawCode.at(i+2)==0x44 && file.rawCode.at(i+3)==0x52){
			
			file.IHDRloc=i+4;		// make the index right after IHDR known
			
		}
	}
}

void findIEND(){
	
	for (int i=0; i<file.rawCode.size();i++){
		// Find IEND chunk location using ascii
		if (file.rawCode.at(i)==0x49 && file.rawCode.at(i+1)==0x45 && file.rawCode.at(i+2)==0x4e && file.rawCode.at(i+3)==0x44){

			file.IENDloc=i+4;		// make the index right after IEND known
			
		}
	}
}

void findIDAT(){
	
		// Find IDAT chunk location using ascii
		for (int i=0;i<file.rawCode.size();i++){
			if (file.rawCode.at(i)==0x49 && file.rawCode.at(i+1)==0x44 && file.rawCode.at(i+2)==0x41 && file.rawCode.at(i+3)==0x54){
				
				file.IDATloc.push_back(i+4);		// make the index on T of IDAT known
												// put into vector becaue there uis more
												// than 1 IDAT
			
		
	}
	}
}

void findPHYS(){
	
	for (int i=0; i<file.rawCode.size();i++){
		// Find pHYs chunk location using ascii
		if (file.rawCode.at(i)==0x70 && file.rawCode.at(i+1)==0x48 && file.rawCode.at(i+2)==0x59 && file.rawCode.at(i+3)==0x73){
			
			file.PHYSloc=i+4;		// make the index right after pHYs known
			
			unsigned char *crcPHYS=new unsigned char [4];
			for (int i=0;i<4;i++){
				int loc=file.PHYSloc-8+i;
				crcPHYS[i]=file.rawCode.at(loc);
			}
			file.crcPHYS=crc(crcPHYS,4);
		}
	}
}

void findCHRM(){
	
	for (int i=0; i<file.rawCode.size();i++){
		// Find cHRM chunk location using ascii
		if (file.rawCode.at(i)==0x63 && file.rawCode.at(i+1)==0x48 && file.rawCode.at(i+2)==0x52 && file.rawCode.at(i+3)==0x4d){
			
			file.CHRMloc=i+4;		// make the index right after cHRM known
			
			unsigned char *crcCHRM=new unsigned char [4];
			for (int i=0;i<4;i++){
				int loc=file.CHRMloc-8+i;
				crcCHRM[i]=file.rawCode.at(loc);
			}
			file.crcCHRM=crc(crcCHRM,4);
		}
	}
}


void IENDinfo(){		// ____IEND____
						//		   ^ up pointer is val of i (file.IENDloc)
	
	// IEND length
	
	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
	
	num1=(file.rawCode.at(file.IENDloc-8) << 24);	// bitshift to the left to add hex
	num2=(file.rawCode.at(file.IENDloc-7) << 16);	// 8 each because each hex is 8 bits
	num3=(file.rawCode.at(file.IENDloc-6) << 8);
	num4= file.rawCode.at(file.IENDloc-5);
	
	// logically add bitshifted values to get correct int from hex
	file.IENDlen =(num1) | (num2) | (num3) | (num4);
	
	unsigned char *crcIEND=new unsigned char [4];
	
	for (int i=0;i<4;i++){
		int loc=file.IENDloc+i;
		crcIEND[i]=file.rawCode.at(loc);
	}
	file.crcIEND=crc(crcIEND,4);
}

void IHDRinfo(){		// ____IHDR____
						//		   ^ up pointer is val of i (file.IHDRloc)
	// IHDR length
	
	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
	
	num1=(file.rawCode.at(file.IHDRloc-8) << 24);	// bitshift to the left to add hex
	num2=(file.rawCode.at(file.IHDRloc-7) << 16);	// 8 each because each hex is 8 bits
	num3=(file.rawCode.at(file.IHDRloc-6) << 8);
	num4= file.rawCode.at(file.IHDRloc-5);
	
	// logically add bitshifted values to get correct int from hex
	file.IHDRlen =(num1) | (num2) | (num3) | (num4);
	
	// Pic Width
	
	int num5,num6,num7,num8;
	
	num5=(file.rawCode.at(file.IHDRloc) << 24);		// bitshift to the left to add hex
	num6=(file.rawCode.at(file.IHDRloc+1) << 16);
	num7=(file.rawCode.at(file.IHDRloc+2) << 8);
	num8= file.rawCode.at(file.IHDRloc+3);
	
	file.width=(num5) | (num6) | (num7) | (num8);
	
	// Pic Height
	
	int num9,num10,num11,num12;
	
	num9=(file.rawCode.at(file.IHDRloc+4) << 24);		// bitshift to the left to add hex
	num10=(file.rawCode.at(file.IHDRloc+5) << 16);
	num11=(file.rawCode.at(file.IHDRloc+6) << 8);
	num12= file.rawCode.at(file.IHDRloc+7);
	
	file.height=(num9) | (num10) | (num11) | (num12);
	
	// Other info only 1 byte each
	
	file.bitdepth=file.rawCode.at(file.IHDRloc+8);
	file.colortype=file.rawCode.at(file.IHDRloc+9);
	file.compMethod=file.rawCode.at(file.IHDRloc+10);
	file.filtMethod=file.rawCode.at(file.IHDRloc+11);
	file.intlMethod=file.rawCode.at(file.IHDRloc+12);

	// crc calc
	
	unsigned char *crcIHDR=new unsigned char [4];
	for (int i=0;i<4;i++){
		int loc=file.IHDRloc+13+i;
		crcIHDR[i]=file.rawCode.at(loc);
	}
	file.crcIHDR=crc(crcIHDR,4);
}

void IDATchunks(){
	
	int num1,num2,num3,num4, len, start ,end,y;
	
	for (int i=0; i<file.IDATloc.size()-1;i++){  // for chunks which arent the last
		
		// get idat chunk data, start 2 bytes after chunk name and end right before CRC
		// __IDAT_____ ... _________IDAT__
		//       ^ S      	   E        ^   , ^=IDATloc and ^=IENDloc, S and E=for loop strt and end pt
		
		// calculate length of chunk
		num1=(file.rawCode.at(file.IDATloc.at(i)-8) << 24);// bitshift to the left to add hex
		num2=(file.rawCode.at(file.IDATloc.at(i)-7) << 16);// 16 each because each hex is 8 bits, and 2 hex
		num3=(file.rawCode.at(file.IDATloc.at(i)-6) << 8);
		num4=(file.rawCode.at(file.IDATloc.at(i)-5) );

		// logically add bitshifted values to get correct int from hex
		len =(num1) | (num2) | (num3) | (num4);
		file.Tlen +=len;							// counts tot length of IDAT
		file.IDATlen.push_back(len);
		
		// attempt to deflate indivudual chunks
		
		start =file.IDATloc.at(i)+1;			// start right after IDAT
		end=file.IDATloc.at(i+1)-9;			// end before next IDAT and crc chunk
		int difference =end-start;			// length of data bytes for each chunk
		
		// make inputs for puff
		unsigned long sourcelen=difference;
		unsigned char *source = new unsigned char [sourcelen]; 			// make array only IDAT data pointer
		unsigned long destlen = file.width*file.height;
		unsigned char *dest = new unsigned char [destlen+6];

		//make source array velue equal to vector
		for (int i= start;i< end;i++){
			y=i-start;
			source[y]=file.rawCode.at(i);
		}
		
		int puffVal= puff(dest, &destlen, source, &sourcelen);
		cout << puffVal  << endl;
		
	}
	
	// for last IDAT chunk
	// calculate length of chunk
	
	long unsigned int e=file.IDATloc.size()-1;
	
	num1=(file.rawCode.at(file.IDATloc.at(e)-8) << 24);// bitshift to the left to add hex
	num2=(file.rawCode.at(file.IDATloc.at(e)-7) << 16);// 16 each because each hex is 8 bits, and 2 hex
	num3=(file.rawCode.at(file.IDATloc.at(e)-6) << 8);
	num4=(file.rawCode.at(file.IDATloc.at(e)-5) );
	
	len =(num1) | (num2) | (num3) | (num4);
	file.Tlen += len; // counts total lenghth of IDAT
	file.IDATlen.push_back(len);
	
	// attempt to deflate indivudual chunks
	
	start =file.IDATloc.at(e)+1;			// start right after IDAT
	end=file.IENDloc-9;					// end before IEND and crc chunk
	int difference =end-start;			// length of data bytes for each chunk
	
	// make inputs for puff
	unsigned long sourcelen=difference;
	unsigned char *source = new unsigned char [sourcelen]; 			// make array pointer
	unsigned long destlen = file.width*file.height;
	unsigned char *dest = new unsigned char [destlen+6];

	//make source array velue equal to vector
	for (int i= start;i< end;i++){
		y=i-start;
		source[y]=file.rawCode.at(i);
	}

	int puffVal= puff(dest, &destlen, source, &sourcelen);
	cout << puffVal <<endl;
}

//void IDATinfo(){
//
//	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
//
//	// calculate length of chunk
//	num1=(file.rawCode.at(file.IDATloc-8) << 24);	// bitshift to the left to add hex
//	num2=(file.rawCode.at(file.IDATloc-7) << 16);	// 8 each because each hex is 8 bits
//	num3=(file.rawCode.at(file.IDATloc-6) << 8);
//	num4= file.rawCode.at(file.IDATloc-5);
//
//	// logically add bitshifted values to get correct int from hex
//	file.IDATlen =(num1) | (num2) | (num3) | (num4);
//
//	// attempt to deflate idat
//
//	 //get idat chunk data, start 2 bytes after chunk name and end right before CRC
//	// __IDAT_____ ... _________IEND__
//	 //      s ^       ^            e  , s=IDATloc, e=IENDloc, ^=for loop strt and end pt
//
//	int start, end;
//	start =file.IDATloc;			// start right after IDAT
//	end=file.IENDloc-9;				// ignore IEND and crc 4 chunk check.
//	int difference =end-start;		// length of data bytes
//	unsigned long sourcelen=difference;
//	unsigned char *source = new unsigned char [sourcelen]; 			// make array pointer
//	unsigned long destlen = file.width*file.height;
//	unsigned char *dest = new unsigned char [destlen+6];
//
//	for (int i= start;i< end;i++){
//		int y=i-start;
//		source[y]=file.rawCode.at(i);
//	}
//
//	int puffVal= puff(dest, &destlen, source, &sourcelen);
//	cout << puffVal <<endl;
//}

void CHRMinfo(){
	
	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
	
	num1=(file.rawCode.at(file.CHRMloc-8) << 24);	// bitshift to the left to add hex
	num2=(file.rawCode.at(file.CHRMloc-7) << 16);	// 8 each because each hex is 8 bits
	num3=(file.rawCode.at(file.CHRMloc-6) << 8);
	num4= file.rawCode.at(file.CHRMloc-5);
	
	// logically add bitshifted values to get correct int from hex
	file.CHRMlen =(num1) | (num2) | (num3) | (num4);
}

void PHYSinfo(){
	
	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
	
	num1=(file.rawCode.at(file.PHYSloc-8) << 24);	// bitshift to the left to add hex
	num2=(file.rawCode.at(file.PHYSloc-7) << 16);	// 8 each because each hex is 8 bits
	num3=(file.rawCode.at(file.PHYSloc-6) << 8);
	num4= file.rawCode.at(file.PHYSloc-5);
	
	// logically add bitshifted values to get correct int from hex
	file.PHYSlen =(num1) | (num2) | (num3) | (num4);
}

void printIHDR(){
	
	cout << "\nIHDR" << "\t\t\t"<<file.IHDRlen << "\t" << file.crcIHDR <<endl;	cout << "width:\t\t\t" << file.width <<endl;
	cout << "height:\t\t\t"<<file.height <<endl;
	cout << "bitdepth:\t\t"<<file.bitdepth <<endl;
	cout << "colortype:\t\t"<<file.colortype <<endl;
	cout << "comp method:\t" <<file.compMethod <<endl;
	cout << "filt method:\t" <<file.filtMethod <<endl;
	cout << "intl method:\t"<<file.intlMethod <<endl;
}

void printPHYS(){
	 
	cout <<"\npHYs\t"<< file.PHYSlen << "\t"<< file.crcPHYS<<endl;
}

void printCHRM(){
	
	cout <<"\ncHRM\t"<< file.CHRMlen << "\t"<< file.crcCHRM<<endl;
}

void printIDAT(){
	
	cout <<"\nIDAT\t" << file.Tlen << "\tCRC" <<endl;
}

void printIEND(){
	
	cout <<"\nIEND\t"<< file.IENDlen << "\t"<< file.crcIEND<<endl;
}

int main()
{
	string fileName;
//	unsigned char a[20000]={};
	cout << "Image Processing Software\n\nSpecify the name of the PNG file that you would like to process.\n>";
	
//	cin >> fileName;
	fileName="brainbow.png";
	
	setInitial();
	openSortFile(fileName);
	findIHDR();			// find loc of IHDR
	IHDRinfo();
	findIDAT();
	findCHRM();
	CHRMinfo();
	findPHYS();
	PHYSinfo();
	findIEND();
	IENDinfo();
	IDATchunks();
	
	printIHDR();
	printPHYS();
	printCHRM();
	printIDAT();
	printIEND();
//	IDATinfo();		// run all first becuase this is dependent on their data, more than 1 idat solving...

	
	

}

