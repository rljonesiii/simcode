/******************************************************
  Robert Jones

  Feb 2, 1997
  CSci 726

  Algorithm C.3
******************************************************/
#include <stdlib.h>
#include <math.h>
#include "rvms.h"
#include "rngs.h"
#include "sort.h"

#define n        7200L
#define k        6
#define alpha    0.05

#define max(x,y) (((x) > (y)) ? (x) : (y))

double log(double x);
double exp(double x);

void main()
{
  long x, i;           /* indices                 */
  long t;              /* rng stream              */
  long *o;             /* histogram               */
  long num_below = 0;
  long num_above = 0;
  long counter;
  long total = 0;
  double f;
  double u;            /* ~Unif(0,1) sample       */
  double r;            /* run length              */
  double p;            /* Pr of run length = x    */
  double *v;           /* chi-square statistic    */
  double v1, v2;
  double diff;
  double F;            /* theoretic cdf           */
  double dt;           /* KS statistic            */
  /* allocate memory */
  o = (long *)calloc(k, sizeof(long));
  o = &o[-1];
  v = (double *)calloc(k, sizeof(double));
  v = &v[-1];
  /*
  *  determine critical values v1 and v2
  */
  v1 = idfChisquare(k-1,alpha/2);
  v2 = idfChisquare(k-1,1-alpha/2);

  printf("\nk = %d, v1 = %g, v2 = %g\n",k,v1,v2);

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
    counter = 0;
    for (i = 1; i <= n; i++) {
      x = 0;
      r = Random();
      counter++;
      do {
        x += 1;
        u = r;
        r = Random();
        counter++;
      } while (r >= u);
      if (x > k) {
        x = k;
      }
      o[x] += 1;
    }
    printf("number of calls to Random = %d\n",counter);
    total += counter;
    /*
    *  compute chi-square statistic
    */
    v[t] = 0.0;
    f = 1.0;
    for (x = 1; x <= k; x++) {
      f *= (x+1);
      p = x/f;
      diff = o[x] - p;
      v[t] += diff*diff/(n*p);
    }
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
  printf("\nKS statistic for 256 tests is %g\n",dt);
  printf("average number of calls to Random = %g\n",(double)total/256);
}/*end*/
