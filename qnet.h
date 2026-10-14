/******************************************************************************
*  File: qnet.h                                                               *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rngs.h"     /* random number generation package        */ 
#include "rvgs.h"     /* functions for random variate generation */
#include "rvms.h"     

typedef enum {OPEN, CLOSED} qnt;
typedef enum {IDLE, BUSY} actvty;

typedef struct {double (*func)();} rv_func;
  
/* internal operations: */

void   Init();
void   Initialize();              
int    GetNextNode(int i); 
double GetMeanRate(int i);

/* user operations: */

void   Network();                           
void   Nodes(int num);                      
void   Connect(int from, int to, double p); 
void   Service(int i, double (*func)(), double);    
void   Arrival(double (*func)(), double);                           
void   InitialState();              
void   Initial(int i, long j);              

