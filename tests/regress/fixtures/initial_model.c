/******************************************************************************
*  initial_model.c -- regression fixture for qnsim.                           *
*  A 2-node open network with feedback that starts with jobs already queued,  *
*  exercising Initial() / InitialState().                                     *
*                                                                             *
*  traffic: l1 = 0.5 + 0.2*l2, l2 = 0.7*l1  =>  l1 ~ 0.58, l2 ~ 0.41          *
*  utilisation: node 1 ~ 0.39 (mean service 2/3), node 2 ~ 0.41 (mean 1.0)   *
*******************************************************************************/
#include "qnet.h"

double arrive(void)   { return Exponential(2.0); }      /* rate 0.5 */
double service1(void) { return Erlang(2, 1.0 / 3.0); }  /* mean 2/3 */
double service2(void) { return Exponential(1.0); }      /* mean 1   */

void Network(void)
{
  Nodes(2);
  Connect(0, 1, 1.0);
  Connect(1, 2, 0.7);
  Connect(1, 0, 0.3);
  Connect(2, 1, 0.2);
  Connect(2, 0, 0.8);
  Arrival(arrive, 0.5);
  Service(1, service1, 1.5);
  Service(2, service2, 1.0);
}

void InitialState(void)
{
  Initial(1, 3);
  Initial(2, 2);
}
