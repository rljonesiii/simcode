/******************************************************************************
*  Module:   qnsim-ss.c                                                       *
*                                                                             *
*  Author:   Rob Jones                                                        *
*                                                                             *
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #3                   *
*            Provides main simulation program for a general-purpose           *
*            STEADY-STATE SIMULATION of a queuing network consisting          *
*            of single server nodes.                                          *
*                                                                             *
*  Version:  5-02-97                                                          *
*                                                                             *
*  Note:     Even though this simulator is capable of simulating an           *
*            open or closed queuing network, the statistics are currently     *
*            applicable for an open network and are referenced to node 0.     *
*            Also, the within-batch sample index (count) is counting the      *
*            number of departures from the system.  If a closed network is    *
*            simulated without using a reference other than node 0, the       *
*            simulation will not terminate.                                   *
*                                                                             *
*******************************************************************************/

#include "qnet.h"
#include "qnsim.h"
#include "queue.h"
#include "misc.h"

/* simulation parameters: */

#define MAX_BATCHES      1000           /* maximum number of batches      */
#define BATCH_SIZE       1000           /* batch size (must be > 1)       */
#define INTERVAL_WIDTH   0.05           /* interval half width in percent */
#define LOC              0.95           /* level of confidence            */
#define INFINITY         1e300          /* a large number                 */


int main(void)
/******************************************************************************
  MAIN PROGRAM
*******************************************************************************/
{
  extern qnt        type;           /* either OPEN or CLOSED                  */
  extern int        num_nodes;      /* number of service nodes                */
  extern rv_func    *random_time;   /* service time random variate functions  */
  extern queue_type *queue;         /* queue for each service node            */
  extern actvty     *status;        /* status of each server: {IDLE,BUSY}     */
  int    node;                      /* service node index                     */
  double *future_events;            /* future-events list                     */
  double clock;                     /* current simulation time                */
  double prev_clock;                /* previous sim time                      */
  double init_clock;                /* time at start of batch                 */
  long   i, j;                      /* general-purpose indices                */
  long   batch;                     /* batch index                            */
  long   count;                     /* sample counter within a batch          */
  long   current, next;             /* service node indices                   */
  long   seed = 123456789;          /* random number seed                     */
  long   n;                         /* number of samples                      */
  double u = 1.0 - 0.5*(1.0 - LOC); /* interval parameter                     */ 
  double s, t, diff;                /* used to compute interval estimate      */
  double area_number;               /* time-averaged number of jobs in system */
  long   num_jobs        = 0;       /* total number of jobs in system         */
  double mean_num_jobs   = 0.0;     /* E[number of jobs in system]            */
  double sumsqr_num_jobs = 0.0;     /* sum of squares to compute variance     */
  double w;                         /* confidence interval half width         */                
  double lambda;                    /* expected external arrival rate         */
  
  /* initialize */

  Network();   /* get specification of queuing network  */
  Init();      /* initialize of network data structures */ 

  /* create future-event list */
  future_events = (double *)calloc(num_nodes,sizeof(double)); 
  if (future_events == NULL) {
    error(MEM_ERR);
  }

  PlantSeeds(seed);
  Initialize();       /* set the initial state of network */
  clock = 0.0;        

  /* schedule first events */
  if (type == OPEN) {
    SelectStream(0);
    future_events[0] = random_time[0].func(); 
  } else {
    future_events[0] = INFINITY; 
  }
  for (i = 1; i < num_nodes; i++) {         
    if (IsEmpty(&queue[i]) == FALSE) {
      num_jobs += Number(&queue[i]);
      ServiceJob(i,clock,future_events);
    } else {
      status[i]        = IDLE;
      future_events[i] = INFINITY;                    
    }
  }

  /* begin simulation */

  for (batch = 1; batch <= MAX_BATCHES; batch++) { 

    /* re-initialize the statistics only */
    init_clock  = clock;
    prev_clock  = clock;
    area_number = 0.0;
    count       = 0;

    while (count < BATCH_SIZE) {

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
        area_number += num_jobs*(clock - prev_clock);
        prev_clock   = clock;
        num_jobs++;     /* one more job in system */
      } else { /* process the departure */    
        if (IsEmpty(&queue[current]) == FALSE) { 
          ServiceJob(current,clock,future_events); 
        } else {
          status[current] = IDLE;
          future_events[current] = INFINITY;
        }
      }
      /* place job in a destination service node */

      next = GetNextNode(current); 

      if (next == 0) { /* if departure from system then */
        area_number += num_jobs*(clock - prev_clock);
        prev_clock   = clock;
        count++;
        num_jobs--;           /* one less job in system */
      } else { 
        ProcessArrival(next,clock,future_events);
      }          
    } /*end while*/ 

    /* compute mean number of jobs in system */
    diff = area_number/(clock-init_clock) - mean_num_jobs;
    sumsqr_num_jobs += diff*diff*(batch - 1.0)/batch;
    mean_num_jobs   += diff/batch;

    /* to reduce some overhead, check on how we're doing every 10 samples */
    if (batch % 10 == 0) {
      n = batch;                    /* the number of samples */
      t = idfStudent(n - 1,u);      /* critical value of t   */
      s = sqrt(sumsqr_num_jobs/n);  /* standard deviation    */
      w = t*s/sqrt(n - 1);          /* interval half width   */
      if (w/mean_num_jobs <= INTERVAL_WIDTH) {
        break; /* exit for loop when desired interval is achieved */
      }
    }
  }/* end for */
  /* 
  *  need to compute width if "for loop" was exited 
  *  because the maximum number of batched were reached                
  */
  if (batch > MAX_BATCHES) {  
    n = MAX_BATCHES;              /* the number of samples */
    t = idfStudent(n - 1,u);      /* critical value of t   */
    s = sqrt(sumsqr_num_jobs/n);  /* standard deviation    */
    w = t*s/sqrt(n - 1);
  }

  lambda = GetMeanRate(0); /* get average external arrival rate */

  /* print results */

  printf("\nSteady-State Simulation\n\n");
  printf("  batch size.........:  %ld\n",BATCH_SIZE);
  printf("  number of batches..:  %ld\n",n);
  printf("  confidence level...:  %g%%\n",LOC*100.0);
  printf("  confidence interval:  +/- %g%%\n\n",INTERVAL_WIDTH*100.0);
  if (w/mean_num_jobs > INTERVAL_WIDTH) {
    printf("Measure did not reach desired interval width.\n\n");
  }
  printf("Expected number of jobs in the system:  %.4f +/- %.4f\n",
          mean_num_jobs, w);
  printf("Expected wait time of a job..........:  %.4f +/- %.4f\n",
          mean_num_jobs/lambda, w/lambda);

  return(0);
}

