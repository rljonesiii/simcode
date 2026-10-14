/******************************************************
  Robert Jones

  Feb 2, 1997
  CSci 726

  Algorithm C.4 gap test 
  
  for Exercise C.7

******************************************************/
#include <stdlib.h>
#include <math.h>
#include "rvms.h"
#include "rngs.h"

#define n        1000L
#define a        0.93
#define b        0.96
#define alpha    0.05

double log(double x);
double exp(double x);

void main()
{
  long x, i, k;        /* indices                 */
  long rns;            /* rng stream              */
  long *o;             /* histogram               */
  long num_below = 0;
  long num_above = 0;
  double u;            /* ~Unif(0,1) sample       */
  double e;            /* expected number of gaps */
  double v, v1, v2;
  double diff;
  const double delta  = b - a;
  const double ndelta = n*delta;
  /*
  *  compute k
  */
  k = 1 + (long)(log(10.0/ndelta)/log(1.0-delta));
  /* 
  *  allocate memory for histogram 
  */
  o = (long *)calloc(k+1, sizeof(long));
  /*
  *  determine critical values v1 and v2
  */
  v1 = idfChisquare(k,alpha/2);
  v2 = idfChisquare(k,1-alpha/2);

  printf("\nk = %d, v1 = %g, v2 = %g\n",k,v1,v2);

  PlantSeeds(231886580);

  /* for each random number stream s */
  for (rns = 0; rns < 256; rns++) {
    SelectStream(rns);
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
      if (0) printf("x=%d\n",x);
    }
    /*
    *  compute chi-square statistic
    */
    v = 0.0;
    for (x = 0; x < k; x++) {
      e = ndelta*pow(1-delta,x);
      diff = o[x] - e;
      v += diff*diff/e;
    }
    /* x = k */
    e = n*pow(1-delta,x);
    diff = o[x] - e;
    v += diff*diff/e;
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
  printf("\nOf the %d tests, %d failed with probability %g,\n",256,num_below+num_above,1-alpha);
  printf("  %d were less than v1 and %d were greater than v2.\n",num_below,num_above);
}/*end*/
