
/* ------------------------------------------------------------------------- * 
 * This program - an extension of program ssq1.c - simulates a single-server * 
 * FIFO service node using Exponentially distributed interarrival times and  * 
 * Uniformly distributed service times (i.e. a M/U/1 queue).                 * 
 *                                                                           * 
 * Name              : ssq2.c  (Single Server Queue, version 2)              * 
 * Author            : Steve Park & Dave Geyer                               * 
 * Language          : ANSI C                                                * 
 * Latest Revision   : 12-09-95                                              * 
 * ------------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>                                             
#include "rng.h"

#define STOP 10000L                             /* number of jobs processed */

/* ============================ */
   static double Exponential(double m)                 /* use m > 0.0 */
/* ============================ */
{                                       
  return (-m * log(1.0 - Random()));     
}

/* ================================== */
   static double Uniform(double a, double b)           /* use a < b */
/* ================================== */
{                                         
  return (a + (b - a) * Random());    
}

/* ============================ */
   static double GetInterarrival(void)
/* ============================ */
{                                               
  return (Exponential(2.0));
}

/* ======================= */
   static double GetService(void)
/* ======================= */
{
  return (Uniform(1.0, 2.0));
}

/* ============== */
   int main(void)
/* ============== */
{
  long   index     = 0;                         /* job index            */
  double interarrival;                          /* interarrival time    */
  double arrival   = 0.0;                       /* time of arrival      */
  double delay;                                 /* delay in queue       */
  double service;                               /* service time         */
  double wait;                                  /* delay + service      */
  double departure = 0.0;                       /* time of departure    */
  struct {                                      /* sum of ...           */
    double delay;                               /*   delay times        */
    double wait;                                /*   wait times         */
    double interarrival;                        /*   interarrival times */
    double service;                             /*   service times      */
  } sum = {0.0, 0.0, 0.0, 0.0};  

  PutSeed(123456789);

  while (index < STOP) {
    index++;
    interarrival      = GetInterarrival();
    arrival          += interarrival;           /* time of arrival      */
    if (arrival < departure)
      delay           = departure - arrival;    /* delay in queue       */
    else
      delay           = 0.0;                    /* no delay             */
    service           = GetService();
    wait              = delay + service;
    departure         = arrival + wait;         /* time of departure    */
    sum.delay        += delay;
    sum.wait         += wait;
    sum.interarrival += interarrival;
    sum.service      += service;
  } 

  printf("\n===> for %ld jobs <===\n", index);
  printf("average interarrival time .. = %6.2f\n", sum.interarrival / index);
  printf("average wait ............... = %6.2f\n", sum.wait / index);
  printf("average delay .............. = %6.2f\n", sum.delay / index);
  printf("average service time ....... = %6.2f\n", sum.service / index);
  printf("average # in the node ...... = %6.2f\n", sum.wait / departure);
  printf("average # in the queue ..... = %6.2f\n", sum.delay / departure);
  printf("utilization ................ = %6.2f\n", sum.service / departure);

  return (0);
}

