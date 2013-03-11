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
int width = 32;
int height = 32;
image_t * image;

void setup (void)
{
        image = image_init();
}

void teardown (void)
{
        image_free(image);
}

void create_test_file(char * path, int r, int g, int b)
{
        width = 512;
        height = 512;
        (*image).path = path;
        (*image).size[0] = width;
        (*image).size[1] = height;
        unsigned char ** pixels = (*image).pixels;
        *pixels = malloc(sizeof(unsigned char) * (*image).size[0]
                         * (*image).size[1] * RGB_CHANNEL);
        for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                        int index = y * width * RGB_CHANNEL + x * RGB_CHANNEL;
                        (*pixels)[index] = r;
                        (*pixels)[index + 1] = g;
                        (*pixels)[index + 2] = b;
                        (*pixels)[index + 3] = 0;
                }
        }
        write_jpeg_image(path, image);
}

START_TEST (test_job_handler)
{
        list_t * files = NULL;
        char * path_1 = "/tmp/test.jpg";
        create_test_file(path_1, 255, 0, 0);
        char * path_2 = "/tmp/test_2.jpg";
        create_test_file(path_2, 0, 0, 255);
        files = list_append(files, path_1);
        files = list_append(files, path_2);
        list_t * similar = process_files(files, 100.f);
        list_t * inner_list = (*similar).value;
        ck_assert_str_eq((*inner_list).value, path_1);
        ck_assert_str_eq((*((*inner_list).next)).value, path_2);
        similar = process_files(files, 1.f);
        fail_unless(similar == NULL, "Expected null result with no match");
}
END_TEST

Suite * jobhandler_suite (void)
{
        Suite *s = suite_create ("job_handler");
        TCase *tc_core = tcase_create ("job_handler");
        tcase_add_checked_fixture (tc_core, setup, teardown);
        tcase_add_test (tc_core, test_job_handler);
        suite_add_tcase (s, tc_core);
        return s;
}

int main (void)
{
        int number_failed;
        Suite *s = jobhandler_suite ();
        SRunner *sr = srunner_create (s);
        srunner_run_all (sr, CK_NORMAL);
        number_failed = srunner_ntests_failed (sr);
        srunner_free (sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
