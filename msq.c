/* ------------------------------------------------------------------------- 
 * This program is a next-event simulation of a multi-server, single-queue 
 * service node.  There are two event types: 
 *         arrival at the service node; 
 *         departure from a server. 
 * The service node is assumed to be initially idle, no arrivals are
 * are permitted after the terminal time 'stop' and the node is then purged 
 * by processing any remaining jobs. 
 * 
 * Name              : msq.c (Multi-Serve Queue)
 * Author            : Steve Park & Dave Geyer 
 * Language          : ANSI C 
 * Latest Revision   : 10-29-96 
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"

#define STOP     20000.0                /* terminal (close the door) time */
#define SERVERS  4                      /* number of servers              */

typedef struct {                        /* the next-event list    */
  double t;                             /*   next event time      */
  int    x;                             /*   event status, 0 or 1 */
} event_list[SERVERS + 1];              

/* ============================ */
   static double Exponential(double m)
/* ============================ */
{
  return (-m * log(1.0 - Random()));
}

/* ================================== */
   static double Uniform(double a, double b)
/* ================================== */
{
  return (a + (b - a) * Random());
}

/* ============================ */                    
   static double GetInterarrival(void)         /* interarrival times have mean 2.0 */
/* ============================ */                    
{     
  SelectStream(0);
  return (Exponential(2.0));
}

/* ======================= */                              
   static double GetService(void)              /* service times have mean 6.0 */ 
/* ======================= */                              
{                 
  SelectStream(1);                            
  return (Uniform(2.0, 10.0));    
}

/* =============================== */
   static int NextEvent(event_list event)
/* =============================== */
{
  int e;                                      
  int i = 0;

  while (event[i].x == 0)
    i++;                         
  e = i;                        /* at this point e is the index of the      */
  while (i < SERVERS) {         /* first 'active' element in the event list */
    i++;                        /* - now check the others                   */
    if ((event[i].x == 1) && (event[i].t < event[e].t))
      e = i;
  }
  return (e);                   /* returns the index of the next event      */
}

/* ============================= */
   static int FindOne(event_list event)
/* ============================= */
{
  int s;
  int i = 1;

  while (event[i].x == 1)
    i++;
  s = i;                        /* at this point s is the index of   */
  while (i < SERVERS) {         /* the first available (idle) server */
    i++;                        /* - now check the others            */
    if ((event[i].x == 0) && (event[i].t < event[s].t))
      s = i;
  }
  return (s);   /* return the index of the available server idle longest */
}

/* ============== */
   int main(void)
/* ============== */
{
  struct {
    double current;                  /* current time                       */
    double next;                     /* next (most imminent) event time    */
  } t;
  event_list event;
  int        e;                      /* next event index                   */
  int        s;                      /* server index                       */
  long       number = 0;             /* number in the node                 */
  long       index  = 0;             /* used to count processed jobs       */
  double     area   = 0.0;           /* time integrated number in the node */
  double     service;
  struct {                           /* accumulated sums of                */
    double service;                  /* - service times                    */
    long   served;                   /* - # served                         */
  } sum[SERVERS + 1];

  PlantSeeds(0);
  t.current    = 0.0;
  event[0].t   = t.current + GetInterarrival();
  event[0].x   = 1;
  for (s = 1; s <= SERVERS; s++) {
    event[s].t     = 0.0;
    event[s].x     = 0;              /* all servers are initially idle */
    sum[s].service = 0.0;
    sum[s].served  = 0;
  }

  while ((event[0].x != 0) || (number != 0)) {
    e         = NextEvent(event);                  /* next event index */
    t.next    = event[e].t;                        /* next event time  */
    area     += (t.next - t.current) * number;     /* update integral  */
    t.current = t.next;                            /* advance the clock*/

    if (e == 0) {                                  /* process an arrival*/
      number++;
      event[0].t        = t.current + GetInterarrival();
      if (event[0].t > STOP)
        event[0].x      = 0;
      if (number <= SERVERS) {
        s               = FindOne(event);
        service         = GetService();
        sum[s].service += service;
        sum[s].served++;
        event[s].t      = t.current + service;
        event[s].x      = 1;
      }
    }
    else {                                         /* process a departure */
      index++;                                     /* from server s       */  
      number--;
      s                 = e;                       
      if (number >= SERVERS) {
        service         = GetService();
        sum[s].service += service;
        sum[s].served++;
        event[s].t      = t.current + service;
      }
      else
        event[s].x      = 0;
    }
  }

  printf("\nfor %ld jobs the node statistics are:\n\n", index);
  printf("  avg interarrivals .. = %6.2f\n", event[0].t / index);
  printf("  avg wait ........... = %6.2f\n", area / index);
  printf("  avg # in node ...... = %6.2f\n", area / t.current);

  for (s = 1; s <= SERVERS; s++)            /* adjust area to calculate */ 
       area -= sum[s].service;              /* averages for the queue   */    

  printf("  avg delay .......... = %6.2f\n", area / index);
  printf("  avg # in queue ..... = %6.2f\n", area / t.current);
  printf("\nthe server statistics are:\n\n");
  printf("    server     utilization     avg service        share\n");
  for (s = 1; s <= SERVERS; s++)
    printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
            sum[s].service / sum[s].served,
            (double) sum[s].served / index);
  printf("\n");

  return (0);
}

