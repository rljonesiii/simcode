/* ------------------------------------------------------------------------- 
 * This program is a next-event simulation of a single-server FIFO service
 * node with immediate feedback using Exponentially distributed interarrival
 * times and service times (i.e., a M/M/1 queue).  
 * There  are 2 events:
 *               arrival of a job; 
 *               completion of service.
 * The simulation clock is advanced by comparing the next scheduled time
 * for each event and then choosing that event which is most imminent.  The
 * service node is assumed to be initially idle, no arrivals are permitted
 * after the terminal time 'stop', and the node is then purged by processing
 * any remaining jobs in the node (including fed back jobs).
 *
 * Name            : simjobs.c 
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 *
 * Additional Implementation: Rob Jones
 *
 * Change History:
 *   Date      Reason
 *   --------  -------------------------------------------------------------
 *
 * Latest Revision : 02-11-97
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rngs.h"                        /* the multi-stream generator */
#include "rvgs.h"
#include "rvms.h"

/* #define PLOT */

#define JOBS      1000        /* terminate after processing this many jobs */
#define MAXREP    1000        /* max replications                          */
#define STOP      20000000.0    /* terminal (close the door) time            */
#define NEVER  (100.0 * STOP)         /* must be much larger than STOP  */
#define BETA      0.0                    /* feedback probability           */

/* ============================== */
   static double Min(double a, double c)
/* ============================== */
{ 
  if (a < c)
    return (a);
  else
    return (c);
} 

/* ======================== */
   static double GetInterarrival()              /* returns an Exponential random */
/* ======================== */           /* variate with mean 1.0         */
{
  SelectStream(0); 
  return (Exponential(1.0));
} 

/* =================== */
   static double GetService()                   /* returns an Exponential random */
/* =================== */                
{
  SelectStream(1);
  return (Exponential(0.8));
} 


/* ================= */
   static int GetFeedback()
/* ================= */
{
  SelectStream(2);
  if (Random() < BETA)
    return (1);
  else
    return (0);
}
  
/* ============== */
   int main(void)
/* ============== */
{
  struct {
    double arrival;                 /* next arrival time                   */
    double completion;              /* next completion time                */
    double current;                 /* current time                        */
    double next;                    /* next (most imminent) event time     */
    double last;                    /* last arrival time                   */
  } t;
  struct {
    double node;                    /* time integrated number in the node  */
    double queue;                   /* time integrated number in the queue */
    double service;                 /* time integrated number in service   */
  } area;
  long i;
  long number;                      /* number in the node                  */
  long index;                       /* used to count departed jobs         */
  long aindex;                      /* used to count arriving jobs         */
  long seed = 123456789;            /* random number generator seed        */
  long rep;                         /* replication index                   */
  double *wait;  
  double delta, w = 0.0;

  wait = (double *)calloc(JOBS + 1,sizeof(double));   /* used as wait[1..JOBS] */
  for (i = 1; i <= JOBS; i++) wait[i] = 0.0;

  PlantSeeds(678912345);
/*
  PlantSeeds(seed);
  PlantSeeds(231886580);
  PlantSeeds(678912345);
*/

  for (rep = 0; rep < MAXREP; rep++) {
    /* 
    *  initialize 
    */
    number       = 0;
    index        = 0; 
    aindex       = 0; 
    area.node    = 0.0;
    area.queue   = 0.0;
    area.service = 0.0;
    t.current    = 0.0;                           /* set the clock            */
    t.arrival    = t.current + GetInterarrival(); /* schedule first arrival   */
    t.completion = NEVER;                      /* first event != completion */

    while (index < JOBS) {
      t.next          = Min(t.arrival, t.completion);  /* next event time   */
      if (number > 0)  {                               /* update integrals  */
        area.node    += (t.next - t.current) * number;
        area.queue   += (t.next - t.current) * (number - 1);
        area.service += (t.next - t.current);
      } 
#ifdef PLOT
#endif
      
      /* advance the clock */
      t.current       = t.next;

      if (t.current >= STOP) {
        break;
      }
  
      if (t.current == t.arrival)  {                   /* process an arrival */

        if (++aindex <= JOBS) {
          wait[aindex] = t.arrival; /* store arrival time for this job */
        }

        number++;
        t.arrival     = t.current + GetInterarrival();

        if (t.arrival > STOP)  {
          t.last      = t.current;
          t.arrival   = NEVER;
        }
        if (number == 1)
          t.completion = t.current + GetService();
        } 
      else {                                          /* process a completion */
        if (GetFeedback())
          t.completion = t.current + GetService();
        else {
          index++;
          number--;

          wait[index] = t.completion - wait[index]; /* store wait time for this job */
          /* average the wait times */
          delta = wait[index] - w;
          w += delta/index;  

          if (number > 0) {
            t.completion = t.current + GetService();
          } else {
            t.completion = NEVER;
          }
         }
      }
    } /*end while*/ 

    /* reset seeds */
    SelectStream(0); GetSeed(&seed); PlantSeeds(seed);

  }/* end for */

#ifdef PLOT
  for (i = 1; i <= index; i++) {
    printf("%f\n", wait[i]); 
  }
#endif
#ifndef PLOT
  printf("\nfor %ld jobs, with feedback probability %4.2f\n", index, BETA);
  printf("   average interarrival time = %6.2f\n", t.current / index);
  printf("   average wait ............ = %6.2f\n", area.node / index);
  printf("   AVERAGE WAIT ............ = %6.2f\n", w);
  printf("   average delay ........... = %6.2f\n", area.queue / index);
  printf("   average service time .... = %6.2f\n", area.service / index);
  printf("   average # in the node ... = %6.2f\n", area.node / t.current);
  printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
  printf("   utilization ............. = %6.2f\n", area.service / t.current);
#endif

  free(wait);
  return (0);
}

