/******************************************************************************
*  Module:   qnet.c                                                           *
*                                                                             *
*  Author:   Rob Jones                                                        *
*                                                                             *
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #3                   *
*            Provides the data structures and operations for the              *
*            construction and simulation of a general queuing network.        *
*                                                                             *
*  Version:  4-14-97                                                          *
*******************************************************************************/

#include "qnet.h"
#include "misc.h"

/* GLOBAL DATA: */

qnt        type         = CLOSED;       /* assume closed network              */ 
int        num_nodes    = 0;            /* number of service nodes            */
queue_type *queue       = nullptr;         /* no. jobs in each service queue     */
actvty     *status      = nullptr;         /* status of each service node        */
double     **P          = nullptr;         /* cumulative prob matrix             */
double     *rates       = nullptr;         /* stores expectations of rates       */
rv_func    *random_time = nullptr;         /* list of functions that return the  */
                                        /* arrival/service time for each node */
static const char Use_Error[]
     = "Must first define the number of nodes with function \"Nodes\"\n";

void Init(void)
/******************************************************************************
  called by main program to initialize data structures
*******************************************************************************/
{
  int i, j;

  if (num_nodes > 0) {
    /* consistency check */
    if (type == OPEN && random_time[0].func == nullptr) {
      error("Must specify an arrival-time function for open networks.");
    }
    for (i = 1; i < num_nodes; i++) {
      if (random_time[i].func == nullptr) {
        error("Must specify a service-time function for every service node.");
      }
    }
    /* compute cumulative probabilities across each row of P matrix */
    for (i = 0; i < num_nodes; i++) {
      for (j = 1; j < num_nodes; j++) {
        P[i][j] += P[i][j-1];
      }
    }
  } else { /* function Network was not used properly */
    error("Must specify queuing network with function \"Network\".\n");
  }
  
  
}

void Initialize(void)
/******************************************************************************
  called by main program to (re)initialize the state of the queuing network
*******************************************************************************/
{
  long i;

  /* empty out each queue */
  for (i = 1; i < num_nodes; i++) {
    FlushQueue(&queue[i]); 
  }
  /* re-initialize each queue */
  InitialState();
}

void Nodes(int n)
/******************************************************************************
  declares the number of service nodes and allocates/initializes 
  the required data structures 
*******************************************************************************/
{
  int i;

  /* constraint checks */
  if (n <= 0) {
    error("Must declare at least 1 service node.\n");
  }
  num_nodes = n + 1;  /* additional node for external I/O */
  /* 
  *  create probability matrix 
  */
  P = (double **)calloc(num_nodes,sizeof(double *)); /* allocate the rows    */
  if (P == nullptr) {
    error(MEM_ERR);
  }
  for (i = 0; i < num_nodes; i++) {                  /* allocate the columns */
    P[i] = (double *)calloc(num_nodes,sizeof(double)); 
    if (P[i] == nullptr) {
      error(MEM_ERR);
    }
  } 
  /* 
  *  create list of random variate functions to call to compute
  *  the arrival/service time of each node
  *  note: calloc initializes random_time[] to null pointers
  */ 
  random_time = (rv_func *)calloc(num_nodes,sizeof(rv_func));
  if (random_time == nullptr) {
    error(MEM_ERR);
  }
  /* 
  *  create list to store the expected values of arrival and service rates
  */
  rates = (double *)calloc(num_nodes,sizeof(double));
  if (rates == nullptr) {
    error(MEM_ERR);
  }
  /* 
  *  create list indicating the status of each service node 
  *  note: calloc initializes status[] to IDLE
  */
  status = (actvty *)calloc(num_nodes,sizeof(actvty));
  if (status == nullptr) {
    error(MEM_ERR);
  }
  /* 
  *  create list of pointers to each service queue 
  */
  queue = (queue_type *)calloc(num_nodes,sizeof(queue_type));
  if (queue == nullptr) {
    error(MEM_ERR);
  }
  /* initialize each queue */
  for (i = 1; i < num_nodes; i++) {
    CreateQueue(&queue[i]); 
  }
}

void Connect(int i, int j, double p)
/******************************************************************************
  user function that connects service node 'i' to node 'j' with probability 'p'
*******************************************************************************/
{
  if (P != nullptr) {
    P[i][j] = p;
  } else {
    error(Use_Error);
  }
}

void Arrival(double (*func)(void), double r)
/******************************************************************************
  user function that specifies the external inter-arrival time function 
  where the parameter 'r' gives the average arrival rate 
*******************************************************************************/
{
  if (random_time != nullptr) {
    type = OPEN;
    random_time[0].func = func;
    rates[0]            = r;
  } else {
    error(Use_Error);
  }
} 

void Service(int i, double (*func)(void), double r)
/******************************************************************************
  user function that assigns a service-time function to node 'i' 
  where the parameter 'r' gives the average rate of service
*******************************************************************************/
{
  if (i <= 0) {
    error("Service node identifiers must be greater than zero.");
  } else if (random_time == nullptr) {
    error(Use_Error);
  } else {
    random_time[i].func = func;
    rates[i]            = r;
  }
}

void Initial(int i, long j)
/******************************************************************************
  user function that initializes service node 'i' with 'j' jobs
*******************************************************************************/
{
  if (queue == nullptr) {
    error(Use_Error);
  } else {

#ifdef SJF_QUEUE /* if SJF then sample/store service time */
    SelectStream(i);
    for (long k = 0; k < j; k++) {
      Insert(&queue[i], random_time[i].func());
    }
#else /* otherwise, just store the number of jobs */
    Insert(&queue[i],j);
#endif
  }
}

int GetNextNode(int i)
/******************************************************************************
  returns the destination node that receives the output of service node 'i'
*******************************************************************************/
{
  int    j;
  double u;

  SelectStream(i);
  u = Random();
  j = 0;
  while (u > P[i][j]) {
    j++;
  }
  return(j);
}

double GetMeanRate(int i)
/******************************************************************************
  returns the average rate of service for node 'i' if 1 <= i <= num_nodes 
  or returns average arrival rate if i = 0
*******************************************************************************/
{
  return(rates[i]);
}

