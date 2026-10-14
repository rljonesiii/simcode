/*
*  Program:      gamble.c
*  Author:       Rob Jones
*  Version:      3/5/97
*
*  Description:  CSCi 726 Homework Exercise 9.3.13
*                Simulation of an absorbing discrete-time birth-death process.
*/  
#include <math.h>
#include <stdio.h>
#include "rngs.h"
#include "rvgs.h"
#include "rvms.h"
/* 
*  simulation parameters:
*/
#define  p          0.51                  /* prob. that player P wins */
#define  k          1000                  /* largest state in chain   */
/* 
*  Since p is close to 0.5, we can replicate 10,000 times 
*  to achieve a +/-0.01 interval with 95% confidence.
*/ 
#define  LOC        0.95                  /* level of confidence      */ 
#define  WIDTH      0.01                  /* desired interval width   */ 
#define  N          10000                 /* number of replications   */    

int main(void)
{
  int    initState[5] = {10,20,30,40,50}; /* possible initial states  */
  int    s;                               /* initial state index      */ 
  int    x;                               /* state of process         */    
  long   i;                               /* replication index        */ 
  long   t;                               /* discrete time t          */    
  long   seed = 123456789;                /* rand. number gen. seed   */
  long   count0;                          /* number of times X(t)=0   */
  long   countk;                          /* number of times X(t)=k   */
  double mtta;                            /* mean time to absorption  */

  SelectStream(0);
  for (s = 0; s < 5; s++) { /* simulate for each possible initial state */
    /* re-initialize */
    count0 = countk = mtta = 0.0;  
    PlantSeeds(seed);
    for (i = 0; i < N; i++) { /* replicate N times */
      t = 0;
      x = initState[s];
      while (x != 0 && x != k) { /* simulate until absorption */
        t++;    /* expected time in state = 1 (a single play) */
        x += 2*Bernoulli(p) - 1;
      }
      if (x == 0) {
        count0++; 
      } else {
        countk++;
      }
      mtta += t;  /* average the times to absorption */
      SelectStream(0); GetSeed(&seed); PlantSeeds(seed); /* use new seed */
    }/* while */
    mtta /= i;
    printf("\nFor initial state a=%d\n",initState[s]);
    printf("with %ld data points and %g%% confidence\n",i,LOC*100);
    printf("    f(0) = %g +/- %.2f\n",(double)count0/i,WIDTH);
    printf("    f(k) = %g +/- %.2f\n",(double)countk/i,WIDTH);
    printf("    Expected Number of Plays = %.1f\n\n",mtta);
  }/* for s */
  printf("\n");
}

/********************************************************************

PROGRAM OUTPUT:

For initial state a=10
with 10000 data points and 95% confidence
    f(0) = 0.6787 +/- 0.01
    f(k) = 0.3213 +/- 0.01
    Expected Number of Plays = 15554.1


For initial state a=20
with 10000 data points and 95% confidence
    f(0) = 0.4447 +/- 0.01
    f(k) = 0.5553 +/- 0.01
    Expected Number of Plays = 26951.6


For initial state a=30
with 10000 data points and 95% confidence
    f(0) = 0.3009 +/- 0.01
    f(k) = 0.6991 +/- 0.01
    Expected Number of Plays = 33431.2


For initial state a=40
with 10000 data points and 95% confidence
    f(0) = 0.2097 +/- 0.01
    f(k) = 0.7903 +/- 0.01
    Expected Number of Plays = 37497.7


For initial state a=50
with 10000 data points and 95% confidence
    f(0) = 0.1392 +/- 0.01
    f(k) = 0.8608 +/- 0.01
    Expected Number of Plays = 40392.2

********************************************************************/
