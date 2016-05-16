/************************************************************************

 QFS decompressor/compressor
 Copyright 2016 Marc Boudreau

 Based on code released by Denis Auroux under the GNU GPL.
 auroux@math.polytechnique.fr
 http://www.math.polytechnique.fr/cmat/auroux/nfs/
 http://auroux.free.fr/nfs/

 This is free software. It is distributed under the terms of the
 GNU General Public License.
 Distributing this software without its source code is illegal.

*************************************************************************/

#include <string.h>
#include <stdlib.h>

// QFS compression quality factor
#define QFS_MAXITER 50  // quick and not so bad

#define WINDOW_LEN (1 << 17)
#define WINDOW_MASK (WINDOW_LEN - 1)

// write_header_block writes the 5 byte header block which consists of the
// magic number 0xFB10 in little endian format and the length of the input
// buffer in big endian format.
void write_header_block(unsigned char *outbuf, int len) {
    outbuf[0] = 0x10;
    outbuf[1] = 0xFB;
    outbuf[2] = len >> 16;
    outbuf[3] = (len >> 8) & 255;
    outbuf[4] = len & 255;
}

// check_redundancy determines if the calculated bestlen and bestoffs fit
// within the QFS algorithm parameters.  The function adjusts the bestlen
// parameter accordingly.  If they don't fit, the bestlen parameter is
// adjusted to 0.
void check_redundancy(int bestoffs, int *bestlen, int inlen, int inpos) {
    if (*bestlen > inlen - inpos) {
        *bestlen = inpos - inlen;
    }

    if (*bestlen <= 2) {
        *bestlen = 0;
    } else if ((*bestlen == 3) && (bestoffs > 1024)) {
        *bestlen = 0;
    } else if ((*bestlen == 4) && (bestoffs > 16384)) {
        *bestlen = 0;
    }
}

// write_non_repeating_blocks encodes sequences of bytes for which a match could
// not be found earlier in the buffer.  The size of each sequence can range from
// 4 to 112 bytes in 4 byte increments.  Sequences are written until there are
// less than 4 unmatchable bytes left in the input buffer.
//
// write_non_repeating_blocks will modify the lastwrot and outpos arguments as it moves through
// the buffers.
//
// write_non_repeating_blocks returns the difference of inpos and the final value of
// lastwrot.
int write_non_repeating_blocks(int inpos, int *lastwrot, unsigned char *outbuf, int *outpos, unsigned char *inbuf) {
    int len;
    int pos = *outpos;
    
    while (inpos - *lastwrot >= 4) {
        len = (inpos - *lastwrot) / 4 - 1;
        
        if (len > 0x1B) {
            len = 0x1B;
        }
        
        outbuf[pos++] = 0xE0 + len;
        len = 4 * len + 4;
        memcpy(outbuf + pos, inbuf + *lastwrot, len);
        
        *lastwrot += len;
        pos += len;
    }
    
    *outpos = pos;
    
    return inpos - *lastwrot;
}

// write_final_block encodes the final 0 to 3 bytes using a special control byte (0xFC
// to 0xFF).  The number of final bytes is encoded in the 3 least significant bits. 
void write_final_block(unsigned char *outbuf, int *outpos, int *len, unsigned char *inbuf, int *lastwrot) {
    int l = *len;
    int pos = *outpos;
    int last = *lastwrot;
    
    outbuf[pos++] = 0xFC + l;

    while (l--) {
        outbuf[pos++] = inbuf[last++];
    }
    
    *len = l;
    *outpos = pos;
    *lastwrot = last;
}

// write_two_control_byte_sequence encodes the bestoffs, bestlen, and len values into
// 2 control bytes.
//
// 0oocccpp oooooooo
//      o: bestoffs
//      c: bestlen
//      p: len 
void write_two_control_byte_sequence(int bestoffs, int bestlen, unsigned char *outbuf, int *outpos, int len) {
    int pos = *outpos;

    outbuf[pos++] = (((bestoffs - 1) >> 8) << 5) + ((bestlen - 3) << 2) + len;
    outbuf[pos++] = (bestoffs - 1) & 0xff;
    
    *outpos = pos;
}

// write_thress_control_byte_sequence encodes the bestoffs, bestlen, and len values into
// 3 control bytes.
//
// 10cccccc ppoooooo oooooooo
//      o: bestoffs
//      c: bestlen
//      p: len
void write_three_control_byte_sequence(int bestoffs, int bestlen, unsigned char *outbuf, int *outpos, int len) {
    int pos = *outpos;
    
    outbuf[pos++] = 0x80 + (bestlen - 4);
    outbuf[pos++] = (len << 6) + ((bestoffs - 1) >> 8);
    outbuf[pos++] = (bestoffs - 1) & 0xff;

    *outpos = pos;
}

// write_four_control_byte_sequence encodes the bestoffs, bestlen, and len values into
// 4 control bytes.
//
// 110occpp oooooooo oooooooo cccccccc
//      o: bestoffs
//      c: bestlen
//      p: len
void write_four_control_byte_sequence(int bestoffs, int bestlen, unsigned char *outbuf, int *outpos, int len) {
    int pos = *outpos;
    
    bestoffs--;
    outbuf[pos++]= 0xC0 + ((bestoffs >> 16) << 4) + (((bestlen - 5) >> 8) << 2) + len;
    outbuf[pos++]=(bestoffs >> 8) & 0xff;
    outbuf[pos++]=bestoffs & 0xff;
    outbuf[pos++]=(bestlen - 5) & 0xff;

    *outpos = pos;
}

// write_compressible_data
void write_compressible_data(int bestoffs, int bestlen, unsigned char *outbuf, int *outpos, int inpos, unsigned char *inbuf, int *lastwrot) {
    int pos = *outpos;
    int last = *lastwrot;

    int len = write_non_repeating_blocks(inpos, &last, outbuf, &pos, inbuf);

    if ((bestlen <= 10) && (bestoffs <= 1024)) {
        write_two_control_byte_sequence(bestoffs, bestlen, outbuf, &pos, len);
    } else if ((bestlen <= 67) && (bestoffs <= 16384)) {
        write_three_control_byte_sequence(bestoffs, bestlen, outbuf, &pos, len);
    } else if ((bestlen <= 1028) && (bestoffs < WINDOW_LEN)) {
        write_four_control_byte_sequence(bestoffs, bestlen, outbuf, &pos, len);
    }
    
    while (len--) {
        outbuf[pos++] = inbuf[last++];
    }
    
    last += bestlen;
    
    *outpos = pos;
    *lastwrot = last;        
}

// compress_data encodes the data provided by the inbuf unsigned char array
// and stores it in the outbuf unsigned char array. If the encoding is
// successful, the function returns 0, any other value indicates an error.
//
// Error codes:
//   1      Insufficient memory to complete
//   2      Encode data is corrupted
int compress_data(unsigned char *inbuf, int inbuflen, unsigned char *outbuf, int *outbuflen)
{
    unsigned char *inrd, *inref, *incmp;
    int *rev_similar; // where is the previous occurrence
    int **rev_last;   // idem
    int offs, len, bestoffs, bestlen, lastwrot, i;
    int inpos, inlen, outpos;
    int *x;

    inlen = inbuflen;
    inpos = 0;
    inrd = inbuf;
    rev_similar = (int *)malloc(4 * WINDOW_LEN);
    rev_last = (int **)malloc(256 * sizeof(int *));

    if (rev_last) {
        rev_last[0] = (int *)malloc(65536 * 4);
    }
  
    if ((outbuf == NULL) || (rev_similar == NULL) || (rev_last == NULL)||(rev_last[0] == NULL)) {
        return 1;
    }
  
    for (i = 1; i < 256; i++) {
        rev_last[i] = rev_last[i - 1] + 256;
    }

    memset(rev_last[0], 0xff, 65536 * 4);
    memset(rev_similar, 0xff, 4 * WINDOW_LEN);

    write_header_block(outbuf, inlen);

    outpos = 5;
    lastwrot = 0;
  
    // main encoding loop
    for (inpos = 0, inrd = inbuf; inpos < inlen; inpos++, inrd++) {
        // This code feeds progress back.
        //if ((inpos & 0x3fff)==0) {
        //    putchar('.');
        //    fflush(stdout);
        //}
    
        // adjust occurrence tables
        x = rev_last[*inrd] + (inrd[1]);
        offs = rev_similar[inpos&WINDOW_MASK] = *x;
        *x = inpos;
    
        // if this has already been compressed, skip ahead
        if (inpos < lastwrot) {
            continue;
        }

        // else look for a redundancy
        bestlen = 0; i = 0;
        while ((offs >= 0) && (inpos - offs < WINDOW_LEN) && (i++ < QFS_MAXITER)) {
            len = 2;
            incmp = inrd + 2;
            inref = inbuf + offs + 2;

            while ((*(incmp++) == *(inref++)) && (len < 1028)) {
                len++;
            }

            if (len > bestlen) {
                bestlen = len;
                bestoffs = inpos - offs;
            }

            offs = rev_similar[offs & WINDOW_MASK];
        }
    
        // check if redundancy is good enough
        check_redundancy(bestoffs, &bestlen, inlen, inpos);

        // update compressed data
        if (bestlen) {
            write_compressible_data(bestoffs, bestlen, outbuf, &outpos, inpos, inbuf, &lastwrot);
        }
    }
  
    // end stuff
    inpos = inlen;
    len = write_non_repeating_blocks(inpos, &lastwrot, outbuf, &outpos, inbuf);
    write_final_block(outbuf, &outpos, &len, inbuf, &lastwrot);

    if (lastwrot!=inlen) {
        return 2;
    }

    *outbuflen = outpos;

    return 0;
}
