#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rngs.h"

int count1 = 0;
int count2 = 0;

static double Triangle(double a, double b, double c)
/*
*  returns a Triangle-distributed random number where
*  a = min value, b = max value, and c = mode
*/
{
  double u, t;
  double gamma = (c-a)/(b-a);

  if ((u = Random()) <= gamma) {
    t = sqrt(u*gamma);
  } else {
    t = 1.0 - sqrt(1.0-gamma+u*(gamma-1.0));
  }
  return(a + (b-a)*t);
}

/* {6.0,11.0,7.0}, {9.0,16.0,11.0}; */
int main(void)
{
  const double a = 9.0; 
  const double b = 16.0; 
  const double c = 11.0; 
  int          i;
  double       t;

  PlantSeeds(123456789);
  SelectStream(0);

  for (i = 0; i < 100000; i++) {
    t = Triangle(a,b,c);
    printf("%g\n", t);
  }
}
