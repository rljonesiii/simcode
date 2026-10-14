/* -------------------------------------------------------------------------  
 * This program is a next-event simulation of a simple inventory system with 
 * a demand process based on a Poisson/Geometric demand model where the
 * demand instances are generated as a Poisson process and at each demand 
 * instance the demand amount is a Geometric random variate.  Backlogging is 
 * possible and when inventory is ordered there is a Uniform(0,1) 
 * delivery lag.   There are 3 events: 
 *           demand on inventory (a demand instance) 
 *           inventory review with an order placed if necessary 
 *           arrival of an order (to replenish inventory). 
 *
 * Name              : sis4.c  (Simple Inventory System, version 4) 
 * Author            : Steve Park & Dave Geyer 
 * Language          : ANSI C 
 * Latest Revision   : 11-10-96 
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"                 /* Random Number GeneratorS  */
#include "rvgs.h"                 /* Random Variate GeneratorS */

#define MINIMUM     20             /* 's' inventory policy parameter >= 0 */
#define MAXIMUM     80             /* 'S' inventory policy parameter > s  */
#define LAMBDA      120.0          /* demand rate, per time interval > 0  */ 
#define PROBABILITY 0.2            /* Pr(demand > 0), per demand instance */
#define STOP        100.0          /* number of time intervals            */
#define NEVER   (100.0 * STOP)


   static double GetInterdemand(void)
/* =============================================== 
 * generate an interdemand time with mean 1/LAMBDA
 * ===============================================
 */
{                                         
   SelectStream(0);
   return (Exponential(1.0 / LAMBDA));
}

   static double GetLag(void)
/* =================================================== 
 * generate a delivery lag uniform between 0.0 and 1.0
 * ===================================================
 */
{                                         
   SelectStream(1);
   return (Uniform(0.0, 1.0));
}

   static long GetDemand(void)                   
/* =============================================================
 * generate a Geometric(p) inventory demand with p = PROBABILITY
 * ============================================================= 
 */
{
   SelectStream(2);
   return (Geometric(PROBABILITY));
}

   static double Min(double a, double b, double c)    
/* ======================================== 
 * returns the minimum of a, b, c 
 * ========================================
 */
{
   double t = a;            

   if (b < t)
     t = b;
   if (c < t)
     t = c;
   return (t);
}


  int main(void)
{
  struct {
    double demand;                        /* next demand time           */
    double review;                        /* next inventory review time */
    double arrive;                        /* next order arrival time    */
    double current;                       /* current time               */
    double next;                          /* next event time            */
  } t;
  struct {                                /* sum of -                */
    double setup;                         /*   setup instances       */
    double holding;                       /*   inventory held (+)    */
    double shortage;                      /*   inventory short (-)   */
    double order;                         /*   orders                */
    double demand;                        /*   demands               */
    double lag;                           /*   lags                  */
  } sum            = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  long inventory   = MAXIMUM;             /* current inventory level */
  long order       = 0;                   /* current order level     */

  PlantSeeds(0);
  t.current = 0.0;                          /* set the clock             */
  t.demand  = t.current + GetInterdemand(); /* schedule the first demand */
  t.review  = t.current + 1.0;              /* schedule the first review */
  t.arrive  = NEVER;                     /* no order arrival pending  */

  while (t.current < STOP) {
    t.next          = Min(t.demand, t.review, t.arrive);
    if (inventory > 0)
      sum.holding  += (t.next - t.current) * inventory;
    else
      sum.shortage -= (t.next - t.current) * inventory;
    t.current       = t.next;

    if (t.current == t.demand) {       /* process an inventory demand */
      long demand;
      demand      = GetDemand();
      sum.demand += demand;
      inventory  -= demand;
      t.demand    = t.current + GetInterdemand();
    }

    else if (t.current == t.review) {  /* process an inventory review */
      if (inventory < MINIMUM) {
        double lag;
        order      = MAXIMUM - inventory;
        sum.setup++;
        sum.order += order;
        lag        = GetLag();
        sum.lag   += lag;
        t.arrive   = t.current + lag;
      }
      t.review  = t.current + 1.0;
    }

    else {                           /* process an inventory arrival*/
      inventory += order;
      order      = 0;
      t.arrive   = NEVER;
    }
  } 

  if (inventory < MAXIMUM) {          /* adjust the final inventory level */
    order      = MAXIMUM - inventory;
    sum.setup++;
    sum.order += order;
    inventory += order;
  }

  printf("\nfor %ld time intervals ", (long) STOP);
  printf("with an average demand of %6.2f\n", sum.demand / STOP);
  if (sum.setup > 0.0)
    printf("an average lag of %4.2f", sum.lag / sum.setup);
  printf(" and policy parameters (s, S) = (%d, %d)\n\n", MINIMUM, MAXIMUM);
  printf("   average order ............ = %6.2f\n", sum.order / STOP);
  printf("   setup frequency .......... = %6.2f\n", sum.setup / STOP);
  printf("   average holding level .... = %6.2f\n", sum.holding / STOP);
  printf("   average shortage level ... = %6.2f\n\n", sum.shortage / STOP);

  return (0);
}
