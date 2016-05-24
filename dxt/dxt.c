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

int score_dxt(unsigned long *px, int nstep, unsigned long col1,
              unsigned long col2, unsigned long *pack)
{
  unsigned char *p1,*p2,*p;
  int vec[3],vdir[3],v2,xa2,xav;
  int i,score,choice;
  
  p1 = (unsigned char *)&col1;
  p2 = (unsigned char *)&col2;
  vdir[0] = (int)p2[0] - (int)p1[0];
  vdir[1] = (int)p2[1] - (int)p1[1];
  vdir[2] = (int)p2[2] - (int)p1[2];
  v2 = vdir[0]*vdir[0] + vdir[1]*vdir[1] + vdir[2]*vdir[2];
  score=0;
  *pack=0;
  p=(unsigned char *)(px+15);
  for (i=15;i>=0;i--,p-=4) {
    vec[0] = (int)p[0] - (int)p1[0];
    vec[1] = (int)p[1] - (int)p1[1];
    vec[2] = (int)p[2] - (int)p1[2];
    xa2 = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
    xav = vec[0]*vdir[0] + vec[1]*vdir[1] + vec[2]*vdir[2];
    if (v2) choice = (nstep*xav+(v2>>1))/v2; else choice = 0;
    if (choice<0) choice = 0;
    if (choice>nstep) choice = nstep;
    score += xa2 - (2*choice*xav)/nstep + (choice*choice*v2)/(nstep*nstep);
    *pack=(*pack)<<2;
    if (choice == nstep) (*pack)++; /* encode: 1 */
    else if (choice) *pack += (choice+1); /* encode: 2 or 3 */
  }
  return score;
}

void pack_dxt(unsigned long *px, unsigned char *dest)
{
  int i1, i2, best_ninter, err, best_err, ncolors;
  unsigned long uniq[16];
  unsigned long col1, col2, best_col1, best_col2, pack;
  unsigned char *c1, *c2;
  unsigned short *sptr, tmpshort;
/* int minc1, minc2, maxc1, maxc2, cc1, cc2, step, channel; */

  /* mark duplicate colors */
  ncolors=0;
  for (i1=0;i1<16;i1++) {
    col1 = px[i1]&0xf8fcf8;
    for (i2=0;i2<ncolors;i2++) if (uniq[i2]==col1) break;
    if (i2==ncolors) uniq[ncolors++] = col1;
  }

  /* optimize over pairs of colors */
  if (ncolors == 1) {
    best_col1 = uniq[0]; best_col2 = uniq[0]; best_err = 1000; best_ninter = 3;
  } else {
    best_err = (1<<30);
    for (i1=0; i1<ncolors-1; i1++)
      for (i2=i1+1; i2<ncolors; i2++) {
        err = score_dxt(px, 2, uniq[i1], uniq[i2], &pack);
        if (err < best_err)
          { best_col1 = uniq[i1]; best_col2 = uniq[i2]; best_ninter = 2; best_err = err; }
        err = score_dxt(px, 3, uniq[i1], uniq[i2], &pack);
        if (err < best_err)
          { best_col1 = uniq[i1]; best_col2 = uniq[i2]; best_ninter = 3; best_err = err; }
      }
  }
  c1 = (unsigned char *)&col1; c2 = (unsigned char *)&col2;

  col1 = best_col1; col2 = best_col2;

  /* finally compress */
  sptr = (unsigned short *)dest;
  sptr[0] = ((unsigned short)c1[0]>>3) + (((unsigned short)c1[1]>>2) << 5)
                                       + (((unsigned short)c1[2]>>3) << 11);
  sptr[1] = ((unsigned short)c2[0]>>3) + (((unsigned short)c2[1]>>2) << 5)
                                       + (((unsigned short)c2[2]>>3) << 11);
  if ((sptr[0] > sptr[1]) ^ (best_ninter == 3))
    { tmpshort=sptr[0]; sptr[0]=sptr[1]; sptr[1]=tmpshort;
      best_col1 = col2; best_col2 = col1; }
  score_dxt(px, best_ninter, best_col1, best_col2, (unsigned long *)(dest+4));
}
