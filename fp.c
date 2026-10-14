
/* check floating point precision (see exercise 4.1.8) */

#include <stdio.h>

typedef float fp;        /* also, with double in place of float */

int main(void)
{
  const fp  ONE   = 1.0;     
        fp  tiny  = ONE;  
        int count = 0;
  
  while(ONE + tiny != ONE) {
    tiny /= 2.0;
    count++;
  }
  printf("count = %d\n", count);
  return (0);
}
