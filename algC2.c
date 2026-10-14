/******************************************************
  Robert Jones

  Feb 2, 1997
  CSci 726

  Algorithm C.2 test of extremes using 
    S = min(U1, U2, ...Ud)
  
  for Exercise C.3

******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rvms.h"
#include "rngs.h"

#define k      100
#define n      (10*k)
#define ndivk  ((double)n/k)      /* expected count per cell (exactly 10) */
#define d      5
#define alpha  0.05

double log(double x);
double exp(double x);

int main(void)
{
  long x, i, j;        /* indices           */
  long rns;            /* rng stream        */
  long *o;             /* histogram         */
  long num_below = 0;
  long num_above = 0;
  double s;            /* min value         */
  double u;            /* ~Unif(0,1) sample */
  double v, v1, v2;
  double diff;
printf("n=%d\n",n);
  /* 
  *  allocate memory for histogram 
  */
  o = (long *)calloc(k + 1, sizeof(long));      /* used as o[1..k] */
  /*
  *  determine critical values v1 and v2
  */
  v1 = idfChisquare(k-1,alpha/2);
  v2 = idfChisquare(k-1,1-alpha/2);
  printf("\nv1 = %g, v2 = %g\n",v1,v2);

  PlantSeeds(231886580);

  /* for each random number stream s */
  for (rns = 0; rns < 256; rns++) {
    SelectStream(rns);
    /*
    *  compute histogram
    */
    for (x = 1; x <= k; x++) {
      o[x] = 0;
    }
    for (i = 1; i <= n; i++) {
      s = 1.0;
      for (j = 1; j <= d; j++) {
        u = Random();
        if (u < s) {
          s = u;
        }
      }
      u = exp(d*log(1-s));
      x = (long)(u*k) + 1;
      o[x] += 1;
    }
    /*
    *  compute chi-square statistic
    */
    v = 0.0;
    for (x = 1; x <= k; x++) {
      diff = o[x] - ndivk;
      v += diff*diff/ndivk;
    }
    /*
    *  pass or fail? 
    */
    if (v < v1) {
      num_below++;
    }
    else if (v > v2) {
      num_above++;
    } 
  }/* end for each stream */
  /*
  *  the test results... 
  */
  printf("\nOf the %d tests, %ld failed with probability %g,\n",256,num_below+num_above,1-alpha);
  printf("  %ld were less than v1 and %ld were greater than v2.\n",num_below,num_above);
}/*end*/
