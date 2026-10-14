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
 * Name            : ssq.c  (Single Server Queue, version 3)
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 10-27-96
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"                        /* the multi-stream generator */

#define TIME      10000.0                /* transient time                 */
#define MAXREP    20000                   /* max replications               */

#define STOP      200000.0                /* terminal (close the door) time */
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

/* ============================ */
   static double Exponential(double m)
/* ============================ */
{
  return (-m * log(1.0 - Random()));
}

/* ================================== */
   [[maybe_unused]] static double Uniform(double a, double b)
/* ================================== */
{
  return (a + (b - a) * Random());
}

/* ======================== */
   static double GetInterarrival()              /* returns an Exponential random */
/* ======================== */           /* variate with mean 2.0         */
{
  SelectStream(0); 
  return (Exponential(1.0));
} 

/* =================== */
   static double GetService()                   /* returns an Exponential random */
/* =================== */                /* variate with mean 0.5         */
{
  SelectStream(1);
  return (Exponential(0.5));
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
  } area      = {0.0, 0.0, 0.0};
  long number = 0;                  /* number in the node                  */
  long index  = 0;                  /* used to count departed jobs        */

  long rep;

  PlantSeeds(123456789);

for (rep = 1; rep <= MAXREP; rep++) {

  number       = 0;
  t.current    = 0.0;                           /* set the clock            */
  t.arrival    = t.current + GetInterarrival(); /* schedule first arrival   */
  t.completion = NEVER;                      /* first event != completion */

  while ((t.arrival < STOP) || (number != 0)) {
    t.next          = Min(t.arrival, t.completion);  /* next event time   */
    if (number > 0)  {                               /* update integrals  */
      area.node    += (t.next - t.current) * number;
      area.queue   += (t.next - t.current) * (number - 1);
      area.service += (t.next - t.current);
    }
    /* advance the clock */
    t.current       = t.next;

    /* check here for transient analysis */
    if (t.current > TIME) {
      printf("%ld\n", number);
      break;
    } 

    if (t.current == t.arrival)  {                   /* process an arrival */
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
        if (number > 0)
          t.completion = t.current + GetService();
        else
          t.completion = NEVER;
       }
    }
  } /*end while*/ 

  /* reset seeds */
  {
    long seed;
    SelectStream(0); GetSeed(&seed); PlantSeeds(seed);
  }

}/* end for */ 
 
  return (0);

  printf("\nfor %ld jobs, with feedback probability %4.2f\n", index, BETA);
  printf("   average interarrival time = %6.2f\n", t.last / index);
  printf("   average wait ............ = %6.2f\n", area.node / index);
  printf("   average delay ........... = %6.2f\n", area.queue / index);
  printf("   average service time .... = %6.2f\n", area.service / index);
  printf("   average # in the node ... = %6.2f\n", area.node / t.current);
  printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
  printf("   utilization ............. = %6.2f\n", area.service / t.current);

  return (0);
}

