/*
*  Robert Jones
*
*  Feb 2, 1997
*  CSci 726
*
*  Exercise C.7
*
*  Algorithm C.4 gap test plus KS statistic
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rvms.h"
#include "rngs.h"
#include "sort.h"

#define n        1000L
#define a        0.93
#define b        0.96
#define alpha    0.05

/* #define PLOT */
#define max(x,y) (((x) > (y)) ? (x) : (y))

int main(void)
{
  long x, i, k;        /* indices                 */
  long t;              /* rng stream              */
  long *o;             /* histogram               */
  long num_below = 0;
  long num_above = 0;
  double u;            /* ~Unif(0,1) sample       */
  double e;            /* expected number of gaps */
  double *v;           /* chi-square statistic    */
  double v1, v2;
  double diff;
  double F;            /* theoretic cdf           */
  double dt;           /* KS statistic            */
  const double delta  = b - a;
  const double ndelta = n*delta;
  /*
  *  compute k
  */
  k = 1 + (long)(log(10.0/ndelta)/log(1.0-delta));
  if (k < 1) {
    fprintf(stderr, "Error: histogram size must be positive.\n");
    return EXIT_FAILURE;
  }

  /* allocate memory */
  o = (long *)calloc(k+1, sizeof(long));
  v = (double *)calloc(256 + 1, sizeof(double)); /* used as v[1..256] */
  /*
  *  determine critical values v1 and v2
  */
  v1 = idfChisquare(k,alpha/2);
  v2 = idfChisquare(k,1-alpha/2);

#ifndef PLOT
  printf("\nk = %ld, v1 = %g, v2 = %g\n",k,v1,v2);
#endif

  PlantSeeds(231886580);

  /* for each random number stream s */
  for (t = 1; t <= 256; t++) {
    SelectStream(t-1);
    /*
    *  compute histogram
    */
    for (x = 0; x <= k; x++) {
      o[x] = 0;
    }
    for (i = 1; i <= n; i++) {
      x = -1;
      do {
        u = Random();
        x += 1;
      } while (u < a || u > b);
      if (x > k) {
        x = k;
      }
      o[x] += 1;
    }
    /*
    *  compute chi-square statistic
    */
    v[t] = 0.0;
    for (x = 0; x < k; x++) {
      e = ndelta*pow(1-delta,x);
      diff = o[x] - e;
      v[t] += diff*diff/e;
    }
    /* x = k */
    e = n*pow(1-delta,x);
    diff = o[x] - e;
    v[t] += diff*diff/e;
  }
  for (i = 1; i <= 256; i++) {
    /*
    *  pass or fail?
    */
    if (v[i] < v1) {
      num_below++;
    }
    else if (v[i] > v2) {
      num_above++;
    }
#ifdef PLOT
    /* plot results of gap test */
    printf("%d %g\n",i,v[i]);
#endif
  }
  /* 
  *  sort chi-square statistics in increasing order 
  */ 
  Sort(v,1,256); 
  /* 
  *  compute KS statistic
  */
  dt = 0.0;
  for (i = 1; i <= 256; i++) {
    F = cdfChisquare(k,v[i]);
    diff = max(fabs(F - (double)(i-1)/t),fabs(F - (double)i/t));
    dt   = max(diff,dt);
  }
#ifndef PLOT
  printf("\nOf the %d tests, %ld failed with probability %g,\n",256,num_below+num_above,1-alpha);
  printf("  %ld were less than v1 and %ld were greater than v2.\n",num_below,num_above);
  printf("KS statistic for 256 tests is %g\n",dt);
#endif
}/*end*/
