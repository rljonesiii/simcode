/*
*  Program:      monte_carlo.c
*  
*  Author:       Rob Jones
*  
*  Version:      3/5/97
*  
*  Description:  Monte Carolo simulation for CSci 726/Exercise 9.3.1 
*/  
#include <math.h>
#include <stdio.h>
#include "rngs.h"
#include "rvgs.h"

#define N 500000  /* number of replications */

#define max(x,y) (((x) > (y)) ? (x) : (y))

int main(void)
{
  long   i, j;    
  double t1, t2, t;
  double mu1, mu2;
  double sum;
  struct {double mu1; double mu2;} parameters[4]   /* ~Expo paramters       */ 
         = {{1,2},{6,10},{7,11},{15,35}};
  double meanT[4]                                  /* theoretic predictions */
         = {2.33,12.25,13.72,39.5}; 

  for (i = 0; i < 4; i++) { 
    mu1 = parameters[i].mu1;
    mu2 = parameters[i].mu2;
    sum = 0.0;
    PlantSeeds(123456789);

    for (j = 0; j < N; j++) { /* do N Monte Carlo replications */
      SelectStream(0);
      t1 = Exponential(mu1);
      SelectStream(1);
      t2 = Exponential(mu2);
      t  = max(t1,t2);
      sum += t;
    }
    printf("mu1 = %-4g  mu2 = %-4g  Theoretical E[T] = %-3.2f \t Sim E[T] = %-.3f \n",
           mu1,mu2,meanT[i],sum/N);
  }
  printf("\n");
}


/*
=====================================================================================
OUTPUT OF PROGRAM
=====================================================================================

mu1 = 1     mu2 = 2     Theoretical E[T] = 2.33          Sim E[T] = 2.334 
mu1 = 6     mu2 = 10    Theoretical E[T] = 12.25         Sim E[T] = 12.253 
mu1 = 7     mu2 = 11    Theoretical E[T] = 13.72         Sim E[T] = 13.726 
mu1 = 15    mu2 = 35    Theoretical E[T] = 39.50         Sim E[T] = 39.509 

=====================================================================================


*/
