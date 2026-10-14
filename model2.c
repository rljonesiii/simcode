/*
*  File:  model2.c  
*
*  Provides the queuing netork description and stochastic specification. 
*/

#include "qnet.h"

static double Expo1(void)
{
  return(Exponential(1.0/90.0));
}

static double Expo2(void)
{
  return(Exponential(1.0/7.0));
}

static double Expo3(void)
{
  return(Exponential(1.0/5.0));
}


void Network(void)
/*
*  define the queuing network
*/
{
  Nodes(3);
  Connect(1,1,0.1);
  Connect(1,2,0.7);
  Connect(1,3,0.2);
  Connect(2,1,1.0);
  Connect(3,1,0.9);
  Connect(3,3,0.1);
  Service(1,Expo1,90.0);        /* mean service rates */
  Service(2,Expo2,7.0);
  Service(3,Expo3,5.0);
}

void InitialState(void)
/*
*  closed network: 10 jobs circulate, all starting at node 1
*/
{
  Initial(1,10);
}

