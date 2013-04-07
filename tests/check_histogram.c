#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <job_handler.h>
#include <cl_util.h>
#include <cl_histogram.h>
#include <image_util.h>
#include <common_test.h>

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
        float results[5];
        float * histo = malloc(sizeof(float) * 32);
        for(unsigned int i = 0; i < 32; i++) {
                histo[i] = 0.1f;
        }
        histogram_average(histo, results, 2);
        for(int i = 0; i < BUCKET_NUMBER; i++) {
                assert_float_equals(results[i], 0.1f);
        }
}
END_TEST

START_TEST (test_histogram_save)
{
        char * test_file = "/tmp/histo_cache";
        histogram_cache_descriptor_init();
        histogram_t *histo = malloc(sizeof(histogram_t));
        histo->file = strdup("test");
        for(unsigned int i = 0; i < BUCKET_NUMBER; i++) {
                histo->results[i] = 0.4f;
        }

        Eina_Hash *map_histo = eina_hash_string_small_new(
                        (void (*)(void *))&histogram_free);
        eina_hash_add(map_histo, "test", histo);
        write_histogram_to_file(test_file, map_histo);
        eina_hash_free(map_histo);

        Eina_Hash *map_histo_2 = read_histogram_file(test_file);
        histogram_t *entry = eina_hash_find(map_histo, "test");
        for(unsigned int i = 0; i < BUCKET_NUMBER; i++) {
                assert_float_equals(entry->results[i], 0.4f);
        }
        write_histogram_to_file(test_file, map_histo_2);

        mark_point();
        eina_hash_free(map_histo);
        histogram_cache_descriptor_shutdown();
}
END_TEST

Suite * histogram_suite (void)
{
        Suite *s = suite_create ("histograms");
        TCase *tc_core = tcase_create ("histograms");
        tcase_add_checked_fixture (tc_core, setup, teardown);
        tcase_add_test (tc_core, test_histogram_distance);
        tcase_add_test (tc_core, test_histogram_average);
        tcase_add_test (tc_core, test_histogram_save);
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
