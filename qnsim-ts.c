/******************************************************************************
*  Module:   qnsim-ts.c                                                       *
*                                                                             *
*  Author:   Rob Jones                                                        *
*                                                                             *
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #3                   *
*            Provides main simulation program for a general-purpose           *
*            TRANSIENT-STATE SIMULATION of a queuing network consisting       *
*            of single server nodes.                                          *
*                                                                             *
*  Version:  5-02-97                                                          *
*******************************************************************************/

#include "qnet.h"
#include "qnsim.h"
#include "misc.h"

/* simulation parameters: */

#define MAX_REPLICATIONS    10000           /* max number of replications     */
#define MAX_EVENTS          100000          /* maximum events per replication */
#define LOC                 0.95            /* level of confidence            */
#define PREC                1.0e-2          /* for rounding data to this prec */
#define NEVER               1e300           /* time of an event that will not occur */


int main(void)
/******************************************************************************
  MAIN PROGRAM
*******************************************************************************/
{
  double *future_events;            /* future-events list                     */
  double clock;                     /* current simulation time                */
  long   i, j;                      /* general-purpose indices                */
  long   index;                     /* replication index                      */
  long   current, next;             /* service node indices                   */
  long   seed = 123456789;          /* random number seed                     */
  long   n;                         /* number of samples                      */
  double u = 1.0 - 0.5*(1.0 - LOC); /* interval parameter                     */ 
  double s, t, diff;                /* used to compute interval estimate      */             
  [[maybe_unused]] long num_jobs; /* total number of jobs in system         */
  long   num_arrivals       = 0;    /* counts # of external job arrivals      */
  long   num_departures     = 0;    /* counts # of external job departures    */
  double arrival_rate;              /* average external arrival rate          */
  double departure_rate;            /* average external departure rate        */
  double mean_arrivals      = 0.0;  /* mean # of arrivals until steady state  */
  double sumsqr_arrivals    = 0.0;  /* sum of squares to compute variance     */
  [[maybe_unused]] double w;        /* confidence interval half width         */                
  
  /* initialize */

  Network();  /* get specification of queuing network  */
  Init();     /* initialize of network data structures */ 

  /* create future-event list */
  future_events = (double *)calloc(num_nodes,sizeof(double)); 
  if (future_events == nullptr) {
    error(MEM_ERR);
  }

  /* begin simulation */

  for (index = 1; index <= MAX_REPLICATIONS; index++) {

    /* re-initialize the state and statistics for each replication */ 

    /* pick a different seed for each replication */
    SelectStream(0); GetSeed(&seed); PlantSeeds(seed);

    Initialize();          /* reset the initial state of network */

    clock           = 0.0;   
    num_arrivals    = 0;
    num_departures  = 0;
    arrival_rate    = 0.0; /* to get started, make arrival rate */
    departure_rate  = 1.0; /*   different from departure_rate   */

    /* schedule first events */
    if (type == OPEN) {
      SelectStream(0);
      future_events[0] = random_time[0].func();
    } else {
      future_events[0] = NEVER;
    }
    num_jobs = 0;
    for (i = 1; i < num_nodes; i++) {
      if (IsEmpty(&queue[i]) == false) {
        num_jobs += Number(&queue[i]);
        ServiceJob(i,clock,future_events);
      } else {
        status[i]        = IDLE;
        future_events[i] = NEVER;
      }
    }
    /* 
    *  To estimate the number of external arrivals before the network
    *  reaches steady state, the stopping criteria "flow in = flow out" 
    *  will be used.  To do the comparison, the average arrival rate 
    *  and average departure rate are both rounded to PREC precision.  
    */
    while ((arrival_rate != departure_rate) && (num_arrivals < MAX_EVENTS)) {

      /* get next (most imminent) event */
      current = 0; 
      for (j = 0; j < num_nodes; j++) {
        if (future_events[j] < future_events[current]) {
          current = j;
        }
      }
      clock = future_events[current]; /* advance the clock */

      /* if event was an outside arrival then schedule a new one */
      if (current == 0) {
        SelectStream(0);
        future_events[current] = clock + random_time[current].func();
        num_arrivals++; 
        num_jobs++;     /* one more job in system */
      } else { /* process the departure */    
        if (IsEmpty(&queue[current]) == false) { 
          ServiceJob(current,clock,future_events); 
        } else {
          status[current] = IDLE;
          future_events[current] = NEVER;
        }
      }
      /* place job in a destination service node */

      next = GetNextNode(current); 

      if (next == 0) { /* if departure from system then */
        num_departures++;
        num_jobs--;           /* one less job in system */
      } else { 
        ProcessArrival(next,clock,future_events);
      }          

      /* compute/round arrival/departure rates */
      arrival_rate   = (double)num_arrivals/clock;
      arrival_rate   = (double)(((long)(arrival_rate/PREC + 0.5))*PREC);
      departure_rate = (double)num_departures/clock;
      departure_rate = (double)(((long)(departure_rate/PREC + 0.5))*PREC);

#ifdef PLOT /* for plotting data */
      printf("%ld \t %f \t %f \t %ld\n",
             num_arrivals,arrival_rate,departure_rate,num_jobs);
#endif

    } /*end while*/ 

    if (num_arrivals == MAX_EVENTS) {
      error("Maximum number of events was reached!\n");
    }

    /* compute mean number of external arrivals until steady state */
    diff = num_arrivals - mean_arrivals;
    sumsqr_arrivals += diff*diff*(index - 1.0)/index;
    mean_arrivals   += diff/index;

  }/* end for */
    
  /* print results */

  n = index-1;                    /* the number of samples */
  t = idfStudent(n - 1,u);        /* critical value of t   */
  s = sqrt(sumsqr_arrivals/n);    /* standard deviation    */
  w = t*s/sqrt(n - 1);            /* interval half width   */

#ifndef PLOT

  printf("\nTransient Simulation:\n\n");
  printf("  sample points......:  %ld\n",n);
  printf("  confidence level...:  %g%%\n\n",LOC*100.0);
  printf("Expected number of arrivals until steady state:  %.4f +/- %.4f\n\n", 
          mean_arrivals,w);
#endif 
  return(0);
}

