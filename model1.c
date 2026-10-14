/******************************************************************************
*  File:  model1.c                                                            *
*                                                                             *
*  Provides the queuing netork description and stochastic specification.      *
*******************************************************************************/

#include "qnet.h"

enum {External, S1, S2, S3} service_nodes;


double Expo0()
{
  return(Exponential(1.0/30.0));
}

double Expo1()
{
  return(Exponential(1.0/90.0));
}

double Expo2()
{
  return(Exponential(1.0/7.0));
}

double Expo3()
{
  return(Exponential(1.0/5.0));
}


void Network()
/******************************************************************************
  define the queuing network
*******************************************************************************/
{
  /* declare 3 service nodes */
  Nodes(3); 

  /* specify the connections */
  Connect(External,S1,1.0);
  Connect(S1,External,0.7);
  Connect(S1,S2,0.2);
  Connect(S1,S3,0.1);
  Connect(S2,External,1.0);
  Connect(S3,External,0.9);
  Connect(S3,S1,0.1);
 
  /* specify a Poisson arrival */
  Arrival(Expo0);

  /* specify service distributions */ 
  Service(S1,Expo1);
  Service(S2,Expo2);
  Service(S3,Expo3);
}

