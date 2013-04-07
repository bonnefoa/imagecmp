#include <math.h>
#define EPSILON 0.01f
#define assert_float_equals(res, expected) \
        fail_unless(fabs(res - expected) < EPSILON\
                        , "Expected %f, got %f", expected, res)
