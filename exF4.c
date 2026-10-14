/****************************************************************************
  File:         urn.c

  Author:       Rob Jones

  Description:  Monte Carlo simulation 

  Last Updated: 11-Feb-97             
****************************************************************************/
#include <math.h>
#include <stdio.h>
#include "rngs.h"
#include "rvgs.h"

#define N 100000L  /* number of replications */

int main(void)
{
  long i;      /* replication index */
  double u, v; /* random variables  */

  for (i = 0; i < N; i++) { /* do N Monte Carlo replications */
    u = Uniform(-1, 1);
    v = u*u*u;
    printf("%f %f\n",u,v);
  }
}

