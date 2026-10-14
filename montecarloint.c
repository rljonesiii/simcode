#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "rngs.h"
#include "rvgs.h"
#include "rvms.h"

#define STANDARD 

// sqrt(2 * pi) 
#define SQRT2PI 2.506628274631    

// confidence interval and level 
double width = 0.01;
double alpha = 0.05;

// integration interval 
double a = 0.0;
double b = 4.0;

   double TruncatedExpo(double m, double c)
/* ====================================================================
 * Returns a truncated exponentially distributed positive real number, 
 * truncated for values greater than "c".
 * NOTE: use mean m > 0.0
 * ====================================================================
 */
{
  return (-m * log(1.0 - (1.0-exp(-c/m))*Random()));
}

// function to be integrated 
double g(double x)
{
  if (x >= a && x <= b) 
    //return exp(-x*x/2)/SQRT2PI;
    return exp(-x*x/2);
  else
    return 0.0;
}

double sample_f()
{
#ifdef STANDARD
  return Uniform(a, b);
#else
  return TruncatedExpo(1.0, b);
#endif
}

double f(double x)
{
  if (x >= 0 && x <= b) 
#ifdef STANDARD
    return 1.0/(b-a);               // Uniform(a,b)
#else
    return exp(-x)/(1.0 - exp(-b)); // truncated expo(1)
#endif
  else
    return 0.0;
}

int main()
{
  double d, t, x, y, ybar, var;
  unsigned int n;

  PlantSeeds(123456789);
  SelectStream(0);

  t    = idfNormal(0.0, 1.0, 1.0-alpha/2.0);
  x    = sample_f();
  y    = g(x)/f(x);
  ybar = y;
  var  = 0.0;
  n    = 1;

  while ((t*sqrt(var/n) > width*sqrt(n-1)) || (n < 40)) {
    n++;
    x = sample_f();
    y = g(x)/f(x);
    d = y-ybar;
    var += d*d*(n-1)/n;
    ybar += d/n;
  }
  printf("n    = %u\n", n);
  printf("ybar = %g\n", ybar);

  return 0;
}
