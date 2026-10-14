#include <stdio.h>
#include <math.h>
#include "rngs.h"              
#include "rvgs.h"
#include "rvms.h"
#include "sort.h"

#define BATCHSIZE      1000           /* batch size           */
#define NUMBATCHES     1000           /* number of batches    */
#define INFINITY       1e300          /* a large number       */

#define ALPHA          1.0          
#define BETA           (1-ALPHA)   

#define Min(a,b)       (a <= b) ? a : b;

int AccessGlobalMem(int p)
{
  SelectStream(p);
  if (Random() < BETA)
    return (1);
  else
    return (0);
}
  
int main(void)
{
  long   i;                 /* batch index                              */
  long   j;                 /* in-batch data index                      */
  long   seed = 123456789;  /* random number generator seed             */
  /* 
  *  initialize 
  */
  PlantSeeds(seed);
  local  = NUM_PROCS;
  queue1 = 0;
  queue2 = 0;
  mem1   = 0;
  mem2   = 0;

  for (i = 1; i <= NUMBATCHES; i++) {
    j  = 0;
    while (j < BATCHSIZE) {
      if (AccessGlobalMem(1)) {
        mem = Equilikely(1,2);  
        if (mem == 1) { 
          if (mem1 == 0) {
            mem1++;
          } else {
            queue1++;
          }
        }
      } 





    } /*end while*/ 
  }/* end for */

  return (0);
}

