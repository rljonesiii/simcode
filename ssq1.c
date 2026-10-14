

/* ------------------------------------------------------------------------- * 
 * This program simulates a single-server FIFO service node using inter-     * 
 * arrival times and service times read from a text file.  The server is     * 
 * assumed to be idle when the first job arrives.  All jobs are processed    *
 * completely so that the server is again idle at the end of the simulation. * 
 * The output statistics are the average interarrival time, average service  * 
 * time, and the average delay in the queue.                                 * 
 *                                                                           * 
 * Name              : ssq1.c  (Single Server Queue, version 1)              * 
 * Authors           : Steve Park & Dave Geyer                               * 
 * Language          : ANSI C                                                * 
 * Latest Revision   : 12-08-95                                              * 
 * ------------------------------------------------------------------------- */

#include <stdio.h>                              

#define FILENAME  "ssq1.dat"                   /* input data file */

/* ============================== */
   double GetInterarrival(FILE *fp)            /* read an interarrival time */
/* ============================== */
{ 
  double r;

  fscanf(fp, "%lf", &r);
  return (r);
}

/* =========================== */
   double GetService(FILE *fp)                 /* read a service time */
/* =========================== */
{ 
  double s;

  fscanf(fp, "%lf\n", &s);
  return (s);
}

/* ============== */
   int main(void)
/* ============== */
{
  FILE   *fp;                                  /* input data file      */
  long   index         = 0;                    /* job index            */
  double interarrival;                         /* interarrival time    */
  double arrival       = 0.0;                  /* arrival time         */
  double delay;                                /* delay in queue       */
  double service;                              /* service time         */
  double wait;                                 /* delay + service      */
  double departure     = 0.0;                  /* departure time       */
  struct {                                     /* sum of ...           */
    double delay;                              /*   delay times        */
    double interarrival;                       /*   interarrival times */
    double service;                            /*   service times      */
  } sum = { 0.0, 0.0, 0.0 };




  fp = fopen(FILENAME, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open input file %s\n", FILENAME);
    return (1);
  }

  while (!feof(fp)) {
    index++;
    interarrival      = GetInterarrival(fp);
    arrival          += interarrival;          /* time of arrival     */
    if (arrival < departure) 
      delay           = departure - arrival;   /* delay in queue      */
    else 
      delay           = 0.0;                   /* no delay            */
    service           = GetService(fp);
    wait              = delay + service;
    departure         = arrival + wait;        /* time of departure   */
    sum.delay        += delay;
    sum.interarrival += interarrival;
    sum.service      += service;
  }

  printf("\n===> for %ld jobs <===\n", index);
  printf("average interarrival time .. = %6.2f\n", sum.interarrival / index);
  printf("average service time ....... = %6.2f\n", sum.service / index);
  printf("average delay .............. = %6.2f\n", sum.delay / index);

  fclose(fp);
  return (0);
}

