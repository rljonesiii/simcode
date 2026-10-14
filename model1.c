/******************************************************************************
*  File:  model1.c                                                            *
*                                                                             *
*  Provides the queuing netork description and stochastic specification.      *
*******************************************************************************/

#include "qnet.h"

enum {External, S1, S2, S3};   /* node names */


static double Expo0(void)
{
  return(Exponential(1.0/30.0));
}

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
  Arrival(Expo0, 30.0);         /* mean arrival rate 30 */

  /* specify service distributions */ 
  Service(S1,Expo1,90.0);       /* mean service rates */
  Service(S2,Expo2,7.0);
  Service(S3,Expo3,5.0);
}

void InitialState(void)
/*
*  the network starts empty
*/
{
}
