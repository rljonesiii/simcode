/******************************************************************************
*  File: qnet.h                                                               *
*******************************************************************************/
#ifndef QNET_H
#define QNET_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rngs.h"     /* random number generation package        */ 
#include "rvgs.h"     /* functions for random variate generation */
#include "rvms.h"
#include "queue.h"

typedef enum {OPEN, CLOSED} qnt;
typedef enum {IDLE, BUSY} actvty;

typedef struct {double (*func)(void);} rv_func;
  
/* network state, defined in qnet.c: */

extern qnt         type;          /* either OPEN or CLOSED                    */
extern int         num_nodes;     /* number of service nodes + 1 (external)   */
extern queue_type *queue;         /* queue for each service node              */
extern actvty     *status;        /* status of each server: {IDLE,BUSY}       */
extern rv_func    *random_time;   /* arrival/service time function per node   */

/* internal operations: */

void   Init(void);
void   Initialize(void);
int    GetNextNode(int i); 
double GetMeanRate(int i);

/* user operations: */

void   Network(void);
void   Nodes(int num);                      
void   Connect(int from, int to, double p); 
void   Service(int i, double (*func)(void), double rate);
void   Arrival(double (*func)(void), double rate);
void   InitialState(void);
void   Initial(int i, long j);

#endif /* QNET_H */
