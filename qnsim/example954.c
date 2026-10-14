/******************************************************************************
*                                                                             *
*  File:  example954.c                                                        * 
*                                                                             *
*  Project #3 Example:                                                        *
*  Queuing netork description and stochastic specification                    *
*  for the 5 node network shown in example 9.5.4 of notes.                    *
*                                                                             *
*******************************************************************************/

#include "qnet.h"

#define lambda    0.83  /* arrival rate    */
#define nu1       0.7   /* service rates...*/
#define nu2       0.8

#ifdef JACKSON

double service1() 
{
  return(Exponential(1.0/nu1));
}
double service2() 
{
  return(Exponential(1.0/nu2));
}

#else

double service1() 
{
  return(Erlang(5,2.0/7.0));
}
double service2()
{
  return(Erlang(4,5.0/16.0));
}

#endif

double Expo()
{
  return(Exponential(1.0/lambda));
}


void Network()
/******************************************************************************
  define the queuing network here
*******************************************************************************/
{
  Nodes(5); /* declare 5 service nodes */

  /* specify the network connections */
  Connect(0,1,0.4);
  Connect(0,3,0.6);
  Connect(1,2,0.8);
  Connect(1,3,0.2);
  Connect(2,0,0.2);
  Connect(2,1,0.4);
  Connect(2,5,0.4);
  Connect(3,4,1.0);
  Connect(4,2,0.2);
  Connect(4,5,0.8);
  Connect(5,0,1.0);

  /* specify the external inter-arrival time */
  Arrival(Expo,lambda);

  /* specify the service times */
  Service(1,service1,nu1);
  Service(2,service2,nu2);
  Service(3,service1,nu1);
  Service(4,service2,nu2);
  Service(5,service2,nu2);
}

void InitialState()
/******************************************************************************
  define the initial state of the queuing network here
*******************************************************************************/
{
  /* network is initially empty */
}

