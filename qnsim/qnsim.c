/******************************************************************************
*  Module:   qnsim.c                                                          *
*                                                                             *
*  Author:   Rob Jones                                                        *
*                                                                             *
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #3                   *
*            Provides auxilary functions for a general-purpose simulation     *
*            of a queuing network consisting of single server nodes.          *
*                                                                             *
*  Version:  5-02-97                                                          *
*                                                                             *
*******************************************************************************/

#include "qnet.h"
#include "queue.h"
#include "misc.h"


void ProcessArrival(long i, double time, double *future_events)
/******************************************************************************
  Auxilary function used to service the arrival of a job to node 'i' at the
  current 'time' if the node is idle, otherwise the job is put into the queue.
  If the job is serviced, the job's departure is also scheduled.
*******************************************************************************/
{
  extern rv_func    *random_time;        /* sample function    */
  extern actvty     *status;             /* server status      */
  extern queue_type *queue;              /* service queue      */
  double job_time   = 0;                 /* job service time   */

  if (status[i] == IDLE) { /* if idle then service job */
    status[i] = BUSY;
    SelectStream(i);
    future_events[i] = time + random_time[i].func();
  } else { /* otherwise, add job to queue */

#ifdef SJF_QUEUE /* if SJF then sample/store service time */

    SelectStream(i);
    job_time = random_time[i].func(); 

#endif

    Insert(&queue[i],job_time);
  }
}


void ServiceJob(long i, double time, double *future_events)
/******************************************************************************
  Auxilary function used to service the next job in the queue of node 'i' at
  the current 'time' thereby scheduling the departure of the job.
*******************************************************************************/
{
  extern rv_func    *random_time;        /* sample function    */
  extern actvty     *status;             /* server status      */
  extern queue_type *queue;              /* service queue      */
  double job_time   = 0;                 /* job service time   */

  Remove(&queue[i],&job_time);

#ifdef SJF_QUEUE /* if SJF then service time is already stored */

  future_events[i] = time + job_time;

#else /* otherwise sample service-time function */

  SelectStream(i);
  future_events[i] = time + random_time[i].func();

#endif

  status[i] = BUSY;
}

