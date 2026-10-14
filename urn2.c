/****************************************************************************

  File:         urn.c

  Author:       Rob Jones

  Description:  Monte Carlo simulation of drawing from an urn initially
                containing n black balls and n white balls.  
                The experiment consists of drawing from the urn twice 
                without replacement.
		
  Last Updated: 11-Feb-97             

****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "rngs.h"

#define PLOT

#define N 1000000L  /* number of replications */

#define n 5        /* number of black balls = number of white balls */

static long Equilikely(long a, long b) /* use a < b */
{
  return (a + (long) ((b - a + 1) * Random()));
}
 
/*
*  The program uses two indices, B and W,
*  and a boundary, 5, to determine the current 
*  number of black and white balls.  The indices
*  are initialized as follows: 
*
*           1 2 3 4 5 6 7 8 9 10
*           b b b b b w w w w w 
*           ^                 ^
*           B                 W  
*/

int main(void)
{
  long i;            /* replication index                      */
  long B;     
  long W;     
  long u;            /* 1 if black on first draw; 0 otherwise  */
  long v;            /* 1 if black on second draw; 0 otherwise */
  long sum_u  = 0;
  long sum_v  = 0;
  long sum_uv = 0;
  double Pu, Pv, Puv;
  double sigma_u, sigma_v;
  double gamma_uv;
  [[maybe_unused]] double rho_uv;
  /* 
  *  do N Monte Carlo replications 
  */
  for (i = 0; i < N; i++) { 
    B = 1; 
    W = 2*n;     
    /* 
    *  draw two balls from urn without replacement 
    */
    if (Equilikely(B, W) <= n) {
      u = 1;
      B++;     /* one less black ball */
    } else {
      u = 0;
      W--;     /* one less white ball */
    }
    if (Equilikely(B, W) <= n) {
      v = 1;
    } else {
      v = 0;
    }
    sum_u  += u;
    sum_v  += v;
    sum_uv += u*v;

#ifdef PLOT
    printf("%ld %ld\n",u,v);
#endif 
  }
  /* 
  *  Compute probabilities of U, V 
  *  same as E[U], E[V] since U, V <-- {0,1}
  */
  Pu  = (double)sum_u/N;
  Pv  = (double)sum_v/N;
  Puv = (double)sum_uv/N; 
  /* 
  *  Compute standard deviations of U, V
  *  note that E[U] = E[U*U] and E[V] = E[V*V]  
  *  since U, V <-- {0,1}
  */
  sigma_u = sqrt(Pu - Pu*Pu);
  sigma_v = sqrt(Pv - Pv*Pv);
  /* 
  *  Compute the covariance, gamma_uv, and
  *  the correlation coefficient, rho_uv, of U,V
  */
  gamma_uv = Puv - Pu*Pv;
  rho_uv   = gamma_uv/(sigma_u*sigma_v);

#ifndef PLOT
  printf("\nBased on %ld replications:\n\n", N);
  printf("E[U]  = Pr{U = 1}  = %g\n",Pu); 
  printf("E[V]  = Pr{V = 1}  = %g\n",Pv); 
  printf("E[UV] = Pr{UV = 1} = %g\n",Puv); 
  printf("sigma_u = %g\n",sigma_u);
  printf("sigma_v = %g\n",sigma_v);
  printf("gamma_uv = %g\n",gamma_uv);
  printf("rho_uv   = %g\n",rho_uv);
  printf("\n");
#endif 
}

