/************************************************************************

 FSH decompressor/compressor
 Copyright 2016 Marc Boudreau

 Based on code released by Denis Auroux under the GNU GPL.
 auroux@math.polytechnique.fr
 http://www.math.polytechnique.fr/cmat/auroux/nfs/
 http://auroux.free.fr/nfs/

 This is free software. It is distributed under the terms of the
 GNU General Public License.
 Distributing this software without its source code is illegal.

*************************************************************************/

void unpack_dxt(unsigned char mask, unsigned short col1, unsigned short col2,
                unsigned char *target);

int score_dxt(unsigned long *px, int nstep, unsigned long col1,
              unsigned long col2, unsigned long *pack);

void pack_dxt(unsigned long *px, unsigned char *dest);                
