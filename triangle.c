
double Triangle(double a, double b, double c)
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


