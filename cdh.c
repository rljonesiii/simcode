/* ------------------------------------------------------------------------- * 
 * This program illustrates an array-based algorithm for tallying a          * 
 * continuous-data histogram for data read from standard input (stdin).      * 
 *                                                                           * 
 * Name              : cdh.c  (Continuous Data Histogram)                    * 
 * Authors           : Steve Park & Dave Geyer                               * 
 * Language          : ANSI C                                                * 
 * Latest Revision   : 03-17-98                                              * 
 *                     command-line argument passing added -- R.L.Jones      *
 * ------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

#define  sqr(x)  ((x) * (x))

#define MIN   (-1/20.0)         /* min value */
#define MAX    1.0               /* max value */
#define K      21                /* number of bins */
#define DELTA  ((MAX - MIN) / K) /* histogram bin size (width) */

#define sqr(x)  ((x) * (x))

/* ============== */
   int main
/* ============== */
  (void)
{
  double x;                                      /* data value          */
  long   j;                                      /* histogram bin index */
  long   index    = 0;                           /* sample size         */
  long   *count;                                 /* bin count           */
  double *midpoint;                              /* bin midpoint        */
  struct {                                       
    long lo;
    long hi;
  } outliers      = {0, 0};
  double sum      = 0.0;
  double sumsqr   = 0.0;
  double mean;
  double stdev;
  FILE   *fp;

  if (K <= 0) {
    printf("Error: Must specify nonzero value for number of bins.\n");
  }

  if ((count = (long *)calloc(K, sizeof(long))) == NULL) {
    printf("\nError allocating memory.\n");
    return EXIT_FAILURE;
  }
  if ((midpoint = (double *)calloc(K, sizeof(double))) == NULL) {
    printf("\nError allocating memory.\n");
    free(count);
    return EXIT_FAILURE;
  }
  fp = fopen("cdh.gpd", "w");                    /* plot data file */
  if (fp == NULL) {
    printf("\nError: unable to create cdh.gpd.\n");
    free(count);
    free(midpoint);
    return EXIT_FAILURE;
  }

  for (j = 0; j < K; j++) 
    midpoint[j] = MIN + (j + 0.5) * DELTA;

  while (!feof(stdin)) {                         /* tally the data */
    scanf("%lf\n", &x);
    index++;
    if ((x >= MIN) && (x < MAX)) {
      j = (long) ((x - MIN) / DELTA);
      count[j]++;
    }
    else if (x < MIN)
      outliers.lo++;
    else
      outliers.hi++;
  }

  for (j = 0; j < K; j++)                        /* histogram mean */
    sum += midpoint[j] * count[j];
  mean   = sum / index;

  for (j = 0; j < K; j++)                        /* histogram stdev */
    sumsqr += sqr(midpoint[j] - mean) * count[j];
  stdev     = sqrt(sumsqr / index);

  printf("  bin        midpoint     count    proportion           density\n\n");
  for (j = 0; j < K; j++) { 
    printf("%5ld", j + 1);                                    /* bin        */
    printf("%16.9f", midpoint[j]);                            /* midpoint   */
    printf("%10ld", count[j]);                                /* count      */
    printf("%14.9f", (double) count[j] / index);              /* proportion */
    printf("%18.9f\n", (double) count[j] / (index * DELTA));  /* density    */
    /* 
    *  print plot information 
    */
    fprintf(fp,"%12.9f", midpoint[j]+DELTA/2.0);                 /* midpoint */
    fprintf(fp,"%12.9f\n", (double) count[j] / (index * DELTA)); /* density  */
  }
  printf("\n");
  printf("sample size .... = %ld\n", index);
  printf("mean ........... = %.9g\n", mean);
  printf("stdev .......... = %.9g\n\n", stdev);
  if (outliers.lo > 0)
    printf("NOTE: there were %ld low outliers\n", outliers.lo);
  if (outliers.hi > 0)
    printf("NOTE: there were %ld high outliers\n", outliers.hi);

  fclose(fp);
  return (0);
}

