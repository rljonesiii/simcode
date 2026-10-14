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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

#define  sqr(x)  ((x) * (x))

[[noreturn]] static void usuage(void)
{
  printf("CONTINUOUS-DATA HISTOGRAM\n");
  printf("usuage : cdh -s # -l # -n #\n");
  printf("  -s # : (int) minimum value\n");
  printf("  -l # : (int) maximum value\n");
  printf("  -n # : (int) number of bins\n\n");
  exit(0);
}

/* ============== */
   int main
/* ============== */
  (int  argc,
   char **argv)
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
  float  MIN = 0.0f;
  float  MAX = 0.0f;
  int    K   = 0;  /* number of bins */
  double DELTA;  /* bin size */
  
  FILE   *fp;

  /* get command line arguments: three "-x value" pairs in any order.
     (2026: rewritten; the 1997 parser stepped past the end of each
     option string into the next argument.) */
  if (argc != 7) {
    usuage();
  }
  for (int a = 1; a < argc; a += 2) {
    const char *opt = argv[a];
    const char *val = argv[a + 1];
    if (opt[0] != '-' || opt[1] == '\0' || opt[2] != '\0'
        || isdigit((unsigned char) val[0]) == 0) {
      usuage();
    }
    switch (opt[1]) {
      case 'n': K   = atoi(val);        break;   /* number of bins */
      case 's': MIN = (float)atoi(val); break;   /* min value      */
      case 'l': MAX = (float)atoi(val); break;   /* max value      */
      default:  usuage();                        /* unknown option */
    }
  }

  if (K <= 0 || MAX <= MIN) {   /* an option was missing or repeated */
    printf("Error: Must specify nonzero value for number of bins.\n");
    usuage();
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

  DELTA = ((MAX - MIN) / K); /* histogram bin size (width) */

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
    if (scanf("%lf\n", &x) != 1)
      break;
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
  free(count);
  free(midpoint);
  return (0);
}

