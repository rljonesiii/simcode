/*
*  Module:       MMck
*
*  Author:       Rob Jones
*
*  Last Updated: 3/5/97
*
*  Description:  CSci 726 Homework Exercise 9.3.10 
*                Computes the state occumpancy probabilities in steady-state 
*                for M/M/c/k service node using Algorithm (9.3.2).  
*/  
#include <math.h>
#include <stdio.h>

#define lambda     8.0      /* arrival rate                       */
#define nu         2.0      /* single server service rate         */
#define c          4        /* max number of servers              */
#define k          10       /* max number allowed in service node */

int main(void)
{
  long   l;                 /* state identifier          */
  double pi[k+1];           /* state probability         */
  double alpha;             /* normalization coefficient */
  double rho;               /* traffic intensity         */
  double fact;              /* factorial                 */
  /* initialization */
  pi[0] = 1.0;              /* initial condition         */
  alpha = 1.0;              /* add pi[0] to alpha        */
  /* 
  *  Compute unnormalized state occupancy probabilities f'(l) = pi 
  *  using Algorithm 9.3.2
  */
  for (l = 1; l <= k; l++) {         
    if (l < c) {
      pi[l] = pi[l-1]*lambda/(l*nu);
    } else {
      pi[l] = pi[l-1]*lambda/(c*nu);
    }
    alpha += pi[l];
  }
  printf("\nDistribution of jobs in M/M/%d/%d service node, f(l)\n",c,k);
  for (l = 0; l <= k; l++) { 
    pi[l] /= alpha;
    printf("f(%ld)\t = %f\n",l,pi[l]);
  }
  printf("\n");
  /*
  *  Perform analytical calculations from Exercise 9.3.9
  */
  rho   = lambda/(c*nu);
  alpha = 1.0;
  fact  = 1.0;
  for (l = 1; l < c; l++) { 
    fact  *= l;
    alpha += pow(c*rho,l)/fact; 
  }
  fact *= c;
  for (l = c; l <= k; l++) { 
    alpha += pow(c*rho,c)*pow(rho,l-c)/fact; 
  }
  printf("\nAnalytical calculation of f(l)\n");
  pi[0] = 1/alpha;
  fact  = 1.0;
  printf("f(%ld)\t = %f\n",0,pi[0]);
  for (l = 1; l < c; l++) { 
    fact *= l;
    pi[l] = pow(c*rho,l)/(alpha*fact); 
    printf("f(%ld)\t = %f\n",l,pi[l]);
  }
  fact *= c;
  for (l = c; l <= k; l++) { 
    pi[l] = pow(c*rho,c)*pow(rho,l-c)/(alpha*fact); 
    printf("f(%ld)\t = %f\n",l,pi[l]);
  }
  printf("\n");
}

=====================================================================================
OUTPUT OF PROGRAM
=====================================================================================

Distribution of jobs in M/M/4/10 service node, f(l)
f(0)     = 0.010169
f(1)     = 0.040678
f(2)     = 0.081356
f(3)     = 0.108475
f(4)     = 0.108475
f(5)     = 0.108475
f(6)     = 0.108475
f(7)     = 0.108475
f(8)     = 0.108475
f(9)     = 0.108475
f(10)    = 0.108475


Analytical calculation of f(l)
f(0)     = 0.010169
f(1)     = 0.040678
f(2)     = 0.081356
f(3)     = 0.108475
f(4)     = 0.108475
f(5)     = 0.108475
f(6)     = 0.108475
f(7)     = 0.108475
f(8)     = 0.108475
f(9)     = 0.108475
f(10)    = 0.108475

=====================================================================================

