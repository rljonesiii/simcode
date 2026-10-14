/****************************************************************************
  File:         urn.c

  Author:       Rob Jones

  Description:  Monte Carlo simulation of drawing from an urn initially
                containing n black balls and n white balls.  
                The experiment consists of drawing from the urn twice 
                without replacement.

  Output:       The random variables 
                  U, the number of black balls {0,1} on first draw and
                  V, the number of black balls {0,1} on second draw 
                are printed to stdout.
		
  Last Updated: 11-Feb-97             
****************************************************************************/
#include <math.h>
#include <stdio.h>
#include "rngs.h"

#define N 1000000L  /* number of replications */
#define n 5         /* number of black balls = number of white balls */

static long Equilikely(long a, long b) /* use a < b */
{
  return (a + (long) ((b - a + 1) * Random()));
}
/*
*  The program uses two indices, B and W, and a boundary, 5, 
*  to determine the current number of black and white balls.  
*  The indices are initialized as follows: 
*
*           1 2 3 4 5 6 7 8 9 10
*           b b b b b w w w w w 
*           ^                 ^
*           B                 W  
*/
int main(void)
{
  long i;    /* replication index                            */
  long B, W; /* see above comment                            */     
  long u;    /* r.v.: 1 if black on first draw; 0 otherwise  */
  long v;    /* r.v.: 1 if black on second draw; 0 otherwise */

  for (i = 0; i < N; i++) { /* do N Monte Carlo replications */
    B = 1; 
    W = 2*n;     
    /* draw first ball from urn without replacement */
    if (Equilikely(B, W) <= n) {
      u = 1;
      B++;     /* one less black ball */
    } else {
      u = 0;
      W--;     /* one less white ball */
    }
    /* draw second ball from urn */
    if (Equilikely(B, W) <= n) {
      v = 1;
    } else {
      v = 0;
    }
    printf("%ld %ld\n",u,v);
  }
}

