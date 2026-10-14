/*
*  Module:   mobile-server.c
*
*  Author:   Rob Jones
*
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #2 
*            Steady-state simulation of a mobile-service queue based 
*            on the paper "Simulation Analysis of Mobile Servers on 
*            a Congested Network" by Kincaid, Miller, and Park, 1992.  
*
*  Input:    The aggregate arrival rate of callers (lambda), 
*            input from command line as a float.
*
*  Output:   Batch means of response time per call,
*            output to stdout as a float.
*
*  Version:  4-02-97 
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rngs.h"              
#include "rvgs.h"
#include "rvms.h"
#include "queue.h"
/* 
*  SIMULATION PARAMETERS:
*/
#define NUM_SERVERS    3              /* # servers                  */
#define NUM_CUSTOMERS  10             /* # customers                */
#define NUM_WEIGHTS    2              /* # unique edge weights      */
#define MEAN_SERVICE   (9*1 + 4*1)    /* E[Erlang(9,1)+Erlang(4,1)] */
#define QUEUE_SIZE     100000         /* sufficiently large queue   */
#define BATCHSIZE      1000           /* batch size (must be > 1)   */
#define NUM_BATCHES    1000           /* number of batches          */
#define LOC            0.95           /* level of confidence        */
#define INFINITY       1e300          /* a large number             */
/* 
*  EXPERIMENTS:   
*
*  define CHECKGRAPH to verify graph connections, 
*  define RESTRICT   to restrict service times to ~Expo(1) and 
*                    travel time to deterministic values; otherwise
*                    service times will be ~Erlang(9,1) + ~Erlang(4,1) 
*                    and travel times will be ~Triangle(a,b,c),                    
*  define LOOKAHEAD  to enable lookahead server scheduling as opposed
*                    to first available scheduling,
*  define HOME_2_4_9 to place service nodes at nodes {2,4,9}; otherwise
*                    servers will be placed at home locations {4,4,4}.
*/
#define CHECKGRAPH
#define HOME_2_4_9 
#define LOOKAHEAD
/*
*  GLOBAL DATA:
*/
typedef enum bool {FALSE = 0, TRUE = 1} BOOL;
/* 
*  the graph topology is hard-wired...
*/
#ifdef RESTRICT /* edges have deterministic delays */

typedef struct { 
  float mean;    /* deterministic travel time */ 
} ParmType;

ParmType parameter[NUM_WEIGHTS] = {2.0, 3.0};

#else /* edges have r.v. delays with ~Triangle(a,b,c) distributions */

typedef struct { /* r.v. edge weights indicate travel time */
  float a, b, c; /* parameters for ~Triangle distribution  */
  float mean;    /* expected value of travel time          */ 
} ParmType;

ParmType parameter[NUM_WEIGHTS] = {{6.0,11.0,7.0,8.0}, {9.0,16.0,11.0,12.0}};

#endif

typedef struct {
  int   edges[NUM_WEIGHTS]; /* the number of edges of each unique weight */
  float delay;              /* (mean) delay along path                   */
} PathType;

#ifdef HOME_2_4_9

int home_locations[NUM_SERVERS] = {2,4,9};

#else /* use {4,4,4} by default */ 

int home_locations[NUM_SERVERS] = {4,4,4};

#endif

PathType shortest_paths[NUM_SERVERS][NUM_CUSTOMERS+1] = {
/* 
*  shortest path connection matrix:  servers X customers 
*/
#ifdef  HOME_2_4_9  /* server home locations = {2,4,9}  */
/*                         CUSTOMERS (CALLERS)                       */
/* SERVER 0:    1           2         3          4          5        */
{{{0,0},0}, {{0,1},0}, {{0,0},0}, {{1,0},0}, {{1,1},0}, {{1,0},0}, 
/*              6           7         8          9          10       */
            {{2,0},0}, {{2,1},0}, {{1,2},0}, {{3,1},0}, {{2,2},0}},
/* SERVER 1:    1           2         3          4          5        */
{{{0,0},0}, {{1,0},0}, {{1,1},0}, {{2,1},0}, {{0,0},0}, {{0,1},0}, 
/*              6           7         8          9          10       */
            {{1,1},0}, {{1,0},0}, {{0,1},0}, {{2,0},0}, {{1,1},0}},
/* SERVER 2:    1           2         3          4          5        */
{{{0,0},0}, {{3,0},0}, {{3,1},0}, {{4,1},0}, {{2,0},0}, {{2,1},0}, 
/*              6           7         8          9          10       */
            {{3,1},0}, {{1,0},0}, {{2,0},0}, {{0,0},0}, {{1,0},0}} 
}; /* note: mean values will be computed later */

#else /* server home locations = {4,4,4} */

/*                         CUSTOMERS (CALLERS)                       */
/* SERVER 0:    1           2         3          4          5        */
{{{0,0},0}, {{1,0},0}, {{1,1},0}, {{2,1},0}, {{0,0},0}, {{0,1},0}, 
/*              6           7         8          9          10       */
            {{1,1},0}, {{1,0},0}, {{0,1},0}, {{2,0},0}, {{1,1},0}},
/* SERVER 1:    1           2         3          4          5        */
{{{0,0},0}, {{1,0},0}, {{1,1},0}, {{2,1},0}, {{0,0},0}, {{0,1},0}, 
/*              6           7         8          9          10       */
            {{1,1},0}, {{1,0},0}, {{0,1},0}, {{2,0},0}, {{1,1},0}},
/* SERVER 2:    1           2         3          4          5        */
{{{0,0},0}, {{1,0},0}, {{1,1},0}, {{2,1},0}, {{0,0},0}, {{0,1},0}, 
/*              6           7         8          9          10       */
            {{1,1},0}, {{1,0},0}, {{0,1},0}, {{2,0},0}, {{1,1},0}}
}; /* note: mean values will be computed later */
#endif

/* the probability distribution of caller requests... */ 
double pdf[NUM_CUSTOMERS+1] = {0, /* index 0 unused */ 
       5.0/30.0, 5.0/30.0, 2.0/30.0, 5.0/30.0, 3.0/30.0, 
       2.0/30.0, 2.0/30.0, 2.0/30.0, 2.0/30.0, 2.0/30.0};
double cdf[NUM_CUSTOMERS+1];
/*
*  STATE INFORMATION:
*/
double    time;                           /* current simulation time    */
double    future_events[NUM_SERVERS+1];   /* future-events list         */
BOOL      busy[NUM_SERVERS];              /* busy status of each server */
/*
*  Allocate a FIFO queue for each server if using LOOKAHEAD
*  or only a single FIFO queue otherwise.  Operations for the queue(s)
*  are located in module "queue.c".  Also, we need to keep track of the 
*  expected queue delay.
*/
#ifdef LOOKAHEAD

double    queue_delay[NUM_SERVERS] = {0.0, 0.0, 0.0};              
QueueType *queues;

#else

double    queue_delay = 0.0;              
QueueType queue; 

#endif

/*
*  FUNCTIONS:
*/
double GetInterarrival(float lambda)              
/*
*  returns ~Poisson distributed arrival events
*/
{
  SelectStream(NUM_SERVERS); 
  return (Exponential(1.0/lambda));
} 

void GetService(int server, int caller, 
                double *travelto, double *travelfrom, double *service) 
/* 
*  returns the travel plus service time for given 'server' and 'caller'
*/
{
  int    i, j, num;
  double to = 0.0, from = 0.0, serv = 0.0;

#ifdef RESTRICT

  to = from = shortest_paths[server][caller].delay; /* delay is deterministic! */
  SelectStream(server);
  serv = Exponential(1.0); /* service time is ~Expo for restricted model */

#else

  SelectStream(server+NUM_SERVERS+1);
  for (i = 0; i < NUM_WEIGHTS; i++) {
    /* 
    *  get number of edges with parameter[i] set and then 
    *  sample ~Triangle distribution for travel time along path
    */ 
    num = shortest_paths[server][caller].edges[i];
    for (j = 0; j < num; j++) {
      to   += Triangle(parameter[i].a, parameter[i].b, parameter[i].c);
      from += Triangle(parameter[i].a, parameter[i].b, parameter[i].c);
    }
  }
  /* service time equals on-site and off-site time */
  SelectStream(server);
  serv = Erlang(9,1.0) + Erlang(4,1.0); 

#endif
  
  (*travelto)   = to;
  (*travelfrom) = from;
  (*service)    = serv;
} 

int GetRequestLocation()
/*
*  returns the location of request based on probability 
*  distribution given by individual arrival rates
*/
{
  int    caller = 1; /* caller (node) index */
  double u;

  SelectStream(NUM_SERVERS+1);
  u = Random();
  while (cdf[caller] <= u) {
    caller++;
  } 
  return(caller);
}

int ChooseServer(int caller)
/*
*  returns the server with the (expected) minimum response time 
*/
{
  int    i; 
  int    server       = 0;
  double response     = INFINITY;
  double min_response = INFINITY;
  /* 
  *  In choosing the best mobile server, consider only the shortest path if not
  *  using LOOOKAHEAD; otherwise consider the busy servers as well by looking
  *  at the expected queue delay and time the server will be available. 
  */
  for (i = 0; i < NUM_SERVERS; i++) {
    if (busy[i] == FALSE) {
      response  = shortest_paths[i][caller].delay;
    }
#ifdef LOOKAHEAD
    else { /* busy server */
      response  = shortest_paths[i][caller].delay;
      response += future_events[i] - time + queue_delay[i];
    } 
#endif
    if (response < min_response) {
      min_response = response;
      server       = i;
    }
  }
  /* 
  *  Return the best server. 
  *  Note: if not using LOOKAHEAD and all servers are busy then this 
  *  function will still return a valid (busy) server (server = 0).
  */
  return(server);
}

int main(int argc, char **argv)
/*
*  main program
*/
{
  const  int arrival = NUM_SERVERS; /* list index for arrival events */
  int    num;                       /* a number                      */	
  int    caller;                    /* customer/caller (node) index  */
  int    server;                    /* server index                  */
  long   batch;                     /* batch index                   */
  long   count;                     /* within-batch job index        */
  long   index;                     /* event list index              */
  long   i, j;                      /* general-purpose indices       */
  long   seed = 123456789;          /* random number generator seed  */
  double lambda = 1.0;              /* aggregate arrival rate        */
  double atime;                     /* request arrival time          */
  double travelto, travelfrom;      /* travel time to/from requester */ 
  double service;                   /* on & off site service time    */
  double sum_response;              /* statistic                     */
  double mean;
  /* 
  *  initialization:
  */
#ifdef LOOKAHEAD
  /* allocate a FIFO queue for each server */
  queues = (QueueType *)calloc(NUM_SERVERS, sizeof(QueueType)); 
  for (i = 0; i < NUM_SERVERS; i++) {
    InitQueue(&queues[i],QUEUE_SIZE);  
  }
#else
  /* a single queue was allocated by declaration, just initialize it */
  InitQueue(&queue,QUEUE_SIZE);  
#endif
  PlantSeeds(seed);
  time = 0.0;                            
  /* schedule first event to be an arrival... */
  future_events[arrival] = time + GetInterarrival(lambda); 
  /* ...not a departure */
  for (i = 0; i < NUM_SERVERS; i++) {         
    future_events[i] = INFINITY;                    
  }
  /* compute mean travel time for each path in graph */
  for (server = 0; server < NUM_SERVERS; server++) {
    for (caller = 1; caller <= NUM_CUSTOMERS; caller++) {
      mean = 0.0;
      for (i = 0; i < NUM_WEIGHTS; i++) {
        /* get the number of edges with same expected value */
        num   = shortest_paths[server][caller].edges[i];
        mean += num*parameter[i].mean; 
      }
      shortest_paths[server][caller].delay = mean; 

#ifdef CHECKGRAPH
      printf("server:%2d = node:(%2d) -- %3g --> (%2d)\n",
              server,home_locations[server],shortest_paths[server][caller].delay,caller);
#endif
    }
  }
#ifdef CHECKGRAPH
  exit(0);
#endif

  /* compute cumulative distribution of caller requests */
  cdf[0] = 0.0;
  for (caller = 1; caller <= NUM_CUSTOMERS; caller++) {
    cdf[caller] = cdf[caller-1] + pdf[caller];
  }
  /* 
  *  get command-line argument = lambda (default = 1.0 if not provided)
  */
  if (argc > 1) {
    lambda = (float)atof(argv[1]);
  }
  /*
  *  begin simulation
  */
  for (batch = 1; batch <= NUM_BATCHES; batch++) { 
    /* 
    *  re-initialize the batch statistics
    */
    count        = 0;
    sum_response = 0.0;
    while (count < BATCHSIZE) {
      /* 
      *  get next (most imminent) event 
      */
      i = arrival; 
      for (j = 0; j < NUM_SERVERS; j++) {
        if (future_events[j] < future_events[i]) {
          i = j;
        }
      }
      time = future_events[i]; /* advance the clock   */
      if (i < NUM_SERVERS)  {  /* process a departure */
        server = i;
#ifdef LOOKAHEAD
        if (NumberInQueue(&queues[server]) > 0) {
          Take(&queues[server],&caller,&atime);
          queue_delay[server] -= 2.0*shortest_paths[server][caller].delay 
                                 + MEAN_SERVICE; 
#else
        if (NumberInQueue(&queue) > 0) {
          Take(&queue,&caller,&atime);
          queue_delay -= 2.0*shortest_paths[server][caller].delay + MEAN_SERVICE; 
#endif
          GetService(server,caller,&travelto,&travelfrom,&service); 
          /* response time = time spent in queue + travel time to request site */
          sum_response += (time - atime) + travelto;  
          count++;
          future_events[server] = time + travelto + travelfrom + service;
          busy[server] = TRUE;
        } else {
          future_events[server] = INFINITY;
          busy[server] = FALSE;
        }
      } else { /* process an arrival */
        caller = GetRequestLocation();   /* where did call originate?      */
        server = ChooseServer(caller);   /* returns the selected server    */
        if (busy[server] == TRUE) {      /* queue caller if server is busy */ 
#ifdef LOOKAHEAD
          Put(&queues[server],caller,time);        
          queue_delay[server] += 2.0*shortest_paths[server][caller].delay 
                                 + MEAN_SERVICE; 
#else
          Put(&queue,caller,time);        
          queue_delay += 2.0*shortest_paths[server][caller].delay + MEAN_SERVICE; 
#endif
        } else { /* else schedule its departure */
          GetService(server,caller,&travelto,&travelfrom,&service); 
          sum_response += travelto; /* no queue delay, just travel! */ 
          count++;
          future_events[server] = time + travelto + travelfrom + service;
          busy[server] = TRUE;
        }
        /* schedule next arrival */ 
        future_events[arrival] = time + GetInterarrival(lambda); 
      }
    } /*end while*/ 

    printf("%f\n",sum_response/BATCHSIZE);

  }/* end for */
  return (0);
}

