/*
*  Program:      gamble-eqn.c
*  Author:       Rob Jones
*  Version:      3/5/97
*
*  Description:  CSCi 726 Homework Exercise 9.3.14
*                Verification of analitical solution for 
*                DTMC in Exercise 9.3.13.
*/  

#include <stdio.h>
#include <math.h>

#define     k   1000        /* largest state in chain                */
#define     p   0.51        /* birth probability, Pr{player P wins}  */
#define     q   (1-p)       /* death probability, Pr{player P loses} */

double N(int j)
{
  if (j == 1) {
    return(1.0);
  } else {
    return(1.0/(double)(1.0-q*p*N(j-1)));
  }
}

int main(void)
{
  int    a, i;
  double prob;

  for (a = 10; a <= 50; a += 10) {
    prob = 1.0;
    for (i = 1; i <= k-a; i++) {
      prob *= N(k-i);
    }
    prob *= pow(p,k-a);
    printf("Pr{a=%d} = %3.2f\n",a,prob);
  }
}

/********************************************************************

PROGRAM OUTPUT:

Pr{a=10} = 0.33
Pr{a=20} = 0.55
Pr{a=30} = 0.70
Pr{a=40} = 0.80
Pr{a=50} = 0.86

********************************************************************/
