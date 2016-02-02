#include <stdio.h>
#include <complex.h>
#include <stdint.h>
#include <String.h>
#include "by_hand.h"

char *foo = "abcxxxxxxxxxxxxxxxxxxxxx";
char afoo[3][2] = { "a", "b", "c" };

int run_vector_test()
{
    float v[3] = {1.f, 2.f, 3.f};
    float s = 6.;
    vector_test_(&s, v);
    return (v[0] == 7. && v[1] == 8. && v[2] == 9.);
}


int run_vector_test2()
{
    float v[3] = {1.f, 2.f, 3.f};
    float s = 6.;
    int32_t n = 2;
    vector_test2_(&s, v, &n);
    return (v[0] == 7. && v[1] == 8. && v[2] == 3.);
}

int run_matrix_test()
{
    float m[3][4] = {
        {1., 2., 3., 4.},
        {1., 2., 3., 4.},
        {1., 2., 3., 4.},        
    };
    
    float a[3][4] = {
        {1., 0., 3., 4.},
        {1., 0., 3., 4.},
        {1., 0., 3., 4.},        
    };
    
    float s = -2.;
    int32_t ncols = 3;
    matrix_test_(&s, m, &ncols);
    return !memcmp(m, a, sizeof(m));
}

int main(int argc, char *argv[])
{
    double x = 2.;
    double y = times2_(&x);

    printf("vector_test %d\n", run_vector_test());
    printf("vector_test2 %d\n", run_vector_test2());
    printf("matrix_test %d\n", run_matrix_test());
    

    printf("TIMES2_(%g) = %g\n", x, y);

    double complex zx = 1. + 2. * I;
    double complex zy = z_times2_(&zx);
    printf("z_times2_(%g + %g*I) = %g + %g*I\n", creal(zx), cimag(zx), creal(zy), cimag(zy));

    int32_t l;
    char c;
    string_test_(&c, &l, foo, 1, 3);
    printf("%c %d\n", c, l);


    float v[3];

    int8_t ichar[3];
    string_array_test_(ichar, afoo, 2);
    for (int i=0; i<3; ++i) {
      printf("%c ", ichar[i]);
    }
    printf("\n");

    return 0;
}

