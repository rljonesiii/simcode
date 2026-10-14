/* ------------------------------------------------------------------------- 
 * This program is a next-event simulation of a single-server FIFO service
 * node with immediate feedback using Exponentially distributed interarrival
 * times and service times (i.e., a M/M/1 queue).  
 *
 * The program provides steady-state statistics using the method of
 * batch means.
 *
 * Name            : ex84x.c
 * Author          : Steve Park & Dave Geyer
 * Additional Impl.: Rob Jones
 * Language        : ANSI C
 *
 * Change History:
 *   Date      Reason
 *   --------  -------------------------------------------------------------
 *   02-18-97  Revised to use method of batch means  
 *
 * Latest Revision : 02-18-97
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"              
#include "rvgs.h"
#include "rvms.h"
#include "sort.h"

#define PLOT
#define _OUTPUT_FOR_ESTIMATE

#define BATCHSIZE      16             /* batch size           */
#define NUMBATCHES     1024           /* number of batches    */
#define LOC            0.95           /* level of confidence  */
#define INFINITY       1e300          /* a large number       */

#define lambda         1.0            /* job arrival rate     */
#define nu             1.25           /* job service rate     */
#define BETA           0.0            /* feedback probability */

int rns;

/* ============================== */
   double Min(double a, double c)
/* ============================== */
{ 
  if (a < c)
    return (a);
  else
    return (c);
} 

/* ======================== */
   double GetInterarrival()              
/* ======================== */          
{
  SelectStream(rns); 
  return (Exponential(1.0/lambda));
} 

/* =================== */
   double GetService()                 
/* =================== */             
{
  SelectStream(rns);
  return (Exponential(1.0/nu));
} 

/* ================= */
   int GetFeedback()
/* ================= */
{
  SelectStream(2);
  if (Random() < BETA)
    return (1);
  else
    return (0);
}
  
/* ============== */
   int sssim(void)
/* ============== */
{
  struct {
    double arrival;         /* next arrival time                        */
    double completion;      /* next completion time                     */
    double current;         /* current time                             */
    double next;            /* next (most imminent) event time          */
    double interval;        /* time width of batch                      */
  } t;
  long   i;                 /* batch index                              */
  long   j;                 /* in-batch data index                      */
  long   number = 0;        /* number in the node                       */
  long   aindex;            /* used to count arriving jobs              */
  long   index;             /* used to count departed jobs              */
  long   seed = 123456789;  /* random number generator seed             */
  double *wait;             /* to gather sample wait times              */
  double aw;                /* batch mean of wait times                 */
  double avg_w, var_w;      /* mean, var of wait times from batch means */
  double delta, u, tstar; 
  /* 
  *  initialize 
  */
  PlantSeeds(seed);
  wait = (double *)calloc(BATCHSIZE*NUMBATCHES,sizeof(double));
  wait = &wait[-1];
  t.current    = 0.0;                           /* set the clock             */
  t.arrival    = t.current + GetInterarrival(); /* schedule first arrival    */
  t.completion = INFINITY;                      /* first event != completion */
  aindex = 0; 
  index  = 0; 
  avg_w  = 0.0;
  var_w  = 0.0;
  /* 
  *  constraint checks 
  */
  if (NUMBATCHES < 2) {
    printf("ERROR: must have at least 2 batch means.\n");
    return (0);
  }

  for (i = 1; i <= NUMBATCHES; i++) {
    /* 
    *  re-initialize the batch statistics
    */
    aw = 0.0;
    j  = 0;

    while (j < BATCHSIZE) {
      t.next = Min(t.arrival, t.completion);  /* next event time    */
      t.current = t.next;                     /* advance the clock  */

      if (t.current == t.arrival)  {          /* process an arrival */
        aindex++;
        /* store arrival time for this job */
        wait[aindex] = t.arrival;
        number++;
        t.arrival = t.current + GetInterarrival();
        if (number == 1)
          t.completion = t.current + GetService();
        } 
      else {                                  /* process a completion */
        if (GetFeedback())
          t.completion = t.current + GetService();
        else {
          index++;
          number--;
          /* store wait time for this job */
          wait[index] = t.completion - wait[index];

          /* Welford's algorithm to compute batch mean of wait times */
          aw += (wait[index] - aw)/(++j);

          if (number > 0)
            t.completion = t.current + GetService();
          else
            t.completion = INFINITY;
         }
      }
    } /*end while*/ 
  /* 
  *  Welford's algorithm to compute mean/variance 
  *  of wait times from batch means...
  */
  delta  = aw - avg_w; 
  var_w += delta*delta*(i-1)/i; 
  avg_w += delta/i;

#ifdef OUTPUT_FOR_ESTIMATE
  printf("%.6f\n",aw); 
#endif

  }/* end for */

  i--;                                   /* i = the number of data points   */

  if (i > 1) {
    u     = 1.0 - 0.5*(1.0 - LOC);       /* interval parameter              */
    tstar = idfStudent(i-1,u);           /* critical value of t             */
  } 
  delta   = tstar*sqrt(var_w/(i*(i-1))); /* half-width of interval estimate */

#ifdef PLOT
  printf("%d %6.4f %6.4f\n", rns, avg_w, delta);
#endif
#ifndef PLOT 
#ifndef OUTPUT_FOR_ESTIMATE 
  printf("\n");
  printf("number of data points =  %ld\n", i);
  printf("batch size........... =  %ld\n", BATCHSIZE); 
  printf("confidence level..... =  %d%%\n", (int)(100.0*LOC+0.5)); 
  printf("arrival rate......... =  %g\n", lambda); 
  printf("service rate......... =  %g\n", nu); 
  printf("feedback prob........ =  %g\n", BETA); 
  printf("\n");
  printf("    AVERAGE WAIT..... = %7.4f +/- %6.4f\n", avg_w, delta);
  printf("\n");
#endif
#endif

  free(&wait[1]);

  return (0);
}


/* ============== */
   int main(void)
/* ============== */
{
  for (rns = 0; rns < 10; rns++) {
    sssim();
  }
  return (0);
}

