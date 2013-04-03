#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <job_handler.h>
#include <cl_util.h>
#include <cl_histogram.h>
#include <image_utils.h>
#include <math.h>
#define EPSILON 0.01f
#define assert_float_equals(res, expected) \
        fail_unless(fabs(res - expected) < EPSILON\
                        , "Expected %f, got %f", expected, res)

void setup (void)
{
}

void teardown (void)
{
}

START_TEST (test_histogram_distance)
{
        float * histo_1 = malloc(sizeof(float) * VECTOR_SIZE);
        float * histo_2 = malloc(sizeof(float) * VECTOR_SIZE);
        mark_point();
        for(unsigned int i = 0; i < 15; i++) {
                histo_1[i] = 0.f;
                histo_2[i] = 0.1f;
        }
        mark_point();
        float res = histogram_distance(histo_1, histo_2);
        assert_float_equals(res, 1.5f);
}
END_TEST

START_TEST (test_histogram_average)
{
        float * histo = malloc(sizeof(float) * 32);
        for(unsigned int i = 0; i < 32; i++) {
                histo[i] = 0.1f;
        }
        float * res = histogram_average(histo, 2);
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                assert_float_equals(res[i], 0.1f);
        }
}
END_TEST

Suite * histogram_suite (void)
{
        Suite *s = suite_create ("cl_image");
        TCase *tc_core = tcase_create ("cl_image");
        tcase_add_checked_fixture (tc_core, setup, teardown);
        tcase_add_test (tc_core, test_histogram_distance);
        tcase_add_test (tc_core, test_histogram_average);
        suite_add_tcase (s, tc_core);
        return s;
}

int main (void)
{
        int number_failed;
        Suite *s = histogram_suite ();
        SRunner *sr = srunner_create (s);
        srunner_run_all (sr, CK_NORMAL);
        number_failed = srunner_ntests_failed (sr);
        srunner_free (sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

