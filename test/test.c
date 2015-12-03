#include <stdio.h>

#include "timestwo.h"

int main(int argc, char **argv)
{
  double a = 3.0;
  double b = 0.0;

  b = times2(&a);

  printf("%f\n", b); 
}
