/*
*  File:  model2.c  
*
*  Provides the queuing netork description and stochastic specification. 
*/

#include "qnet.h"

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
  Service(1,Expo1);
  Service(2,Expo2);
  Service(3,Expo3);
  Initial(1,10);
}

