/* -------------------------------------------------------------------------  
 * This program is a next-event simulation of a simple inventory system with 
 * a Poisson demand process, backlogging, and a Uniform(0,1) delivery lag. 
 * There are 3 events: 
 *           demand on inventory 
 *           review inventory and place an order if necessary 
 *           arrival of an order (to replenish inventory). 
 *
 * Name              : sis3.c  (Simple Inventory System, version 3) 
 * Author            : Steve Park & Dave Geyer 
 * Language          : ANSI C 
 * Latest Revision   : 10-29-95 
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"

#define MINIMUM    20             /* 's' inventory policy parameter >= 0 */
#define MAXIMUM    80             /* 'S' inventory policy parameter > s  */
#define STOP       100.0
#define NEVER   (100.0 * STOP)

/* ============================ */
   static double Exponential(double m)
/* ============================ */
{
  return (-m * log(1.0 - Random()));
}

/* ================================== */
   static double Uniform(double a, double b)                         
/* ================================== */
{                                  
  return (a + (b - a) * Random());  
}

/* =========================== */
   static double GetInterdemand(void)
/* =========================== */
{                                         
  SelectStream(0);                        /* generate an interdemand time */
  return (Exponential(0.03333333));       /* with mean 1 / 30             */
}

/* =================== */
   static double GetLag(void)
/* =================== */
{                                         
  SelectStream(1);                        /* generate a delivery lag time  */
  return (Uniform(0.0, 1.0));             /* uniform between 0 and 1 and   */
}

/* ======================================== */
   static double Min(double a, double b, double c)    
/* ======================================== */
{                                     /* returns the minimum of a, b, c */
  double t = a;            

  if (b < t)
    t = b;
  if (c < t)
    t = c;
  return (t);
}

/* ============== */
   int main(void)
/* ============== */
{
  struct {
    double demand;                        /* next demand time                */
    double review;                        /* next inventory review time      */
    double arrive;                        /* next order arrival time         */
    double current;                       /* current time                    */
    double next;                          /* next (most imminent) event time */
  } t;
  struct {                                /* sum of -                */
    double setup;                         /*   setup instances       */
    double holding;                       /*   inventory held (+)    */
    double shortage;                      /*   inventory short (-)   */
    double order;                         /*   orders                */
    double demand;                        /*   demands               */
    double lag;                           /*   lags                  */
  } sum            = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double lag;
  long   inventory = MAXIMUM;             /* current inventory level */
  long   order     = 0;                   /* current order level     */

  PlantSeeds(0);
  t.current = 0.0;
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
      sum.demand++;
      inventory--;
      t.demand = t.current + GetInterdemand();
    }

    else if (t.current == t.review) {  /* process inventory review */
      if (inventory < MINIMUM) {
        order      = MAXIMUM - inventory;
        sum.setup++;
        sum.order += order;
        lag        = GetLag();
        sum.lag   += lag;
        t.arrive   = t.current + lag;
      }
      t.review  = t.current + 1.0;
    }

    else {                           /* process an inventory order arrival*/
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

