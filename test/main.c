#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include "by_hand.h"

char *foo = "abcxxxxxxxxxxxxxxxxxxxxx";
char afoo[3][2] = { "a", "b", "c" };

int main(int argc, char *argv[])
{
    double x = 2.;
    double y = times2_(&x);

    printf("TIMES2_(%g) = %g\n", x, y);

    double complex zx = 1. + 2. * I;
    double complex zy = z_times2_(&zx);
    printf("z_times2_(%g + %g*I) = %g + %g*I\n", creal(zx), cimag(zx), creal(zy), cimag(zy));

    int32_t l;
    char c;
    string_test_(&c, &l, foo, 1, 3);
    printf("%c %d\n", c, l);


    float ax[3][2];
    array_test1_(ax);
    for (int row=0; row<2; ++row) {
        for (int col=0; col<3; ++col) {
            printf("%g ", ax[col][row]);
        }
    }
    printf("\n");

    int8_t ichar[3];
    string_array_test_(ichar, afoo, 2);
    for (int i=0; i<3; ++i) {
      printf("%c ", ichar[i]);
    }
    printf("\n");

    
    return 0;
}

