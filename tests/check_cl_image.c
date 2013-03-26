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
clinfo_t * clinfo;
job_t * job;
image_t * image;

void setup (void)
{
        job = job_init();
        image = image_init();
        job->image = image;
        image->image_fmt->image_channel_order = CL_RGBA;
        image->image_fmt->image_channel_data_type = CL_UNSIGNED_INT8;
        image->path = "test_image";
        clinfo = clinfo_init("src/kernel_image.cl", "generate_histogram");
}

void teardown (void)
{
        clinfo_free(clinfo);
        job_free(job);
}

void fill_rgba_pixels(unsigned char (*fill_funct)(int, int, int))
{
        unsigned char ** pixels = image->pixels;
        *pixels = malloc(sizeof(unsigned char) * width * height * RGBA_CHANNEL);
        for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                        int index = y * width * RGBA_CHANNEL + x * RGBA_CHANNEL;
                        (*pixels)[index] = (*fill_funct)(x, y, 0);
                        (*pixels)[index + 1] = (*fill_funct)(x, y, 1);
                        (*pixels)[index + 2] = (*fill_funct)(x, y, 2);
                        (*pixels)[index + 3] = 0;
                }
        }
        image->path = "test";
        image->size[0] = width;
        image->size[1] = height;
}

unsigned char zero_fill(int x, int y, int c)
{
        (void)x;
        (void)y;
        (void)c;
        return 0;
}

unsigned char blue_green_fill(int x, int y, int c)
{
        (void)x;
        (void)y;
        switch (c) {
        case 0 :
                return 1;
        case 1 :
                return 100;
        case 2 :
                return 250;
        }
        return 0;
}

unsigned char spilled_fill(int x, int y, int c)
{
        (void)x;
        (void)y;
        switch (c) {
        case 0 :
                if (x < VECTOR_SIZE)
                        return 0;
                return 254;
        case 1 :
                if (x < VECTOR_SIZE)
                        return 51;
                return 204;
        case 2 :
                if (x < VECTOR_SIZE)
                        return 102;
                return 153;
        }
        return 0;
}

void check_blue_green_results(float * results)
{
        assert_float_equals(results[0 * BUCKET_NUMBER], 1.f);
        assert_float_equals(results[1 * BUCKET_NUMBER + 1], 1.f);
        assert_float_equals(results[2 * BUCKET_NUMBER + 4], 1.f);
}

START_TEST (test_near_power)
{
        ck_assert_int_eq(round_up_power_of_two(3), 4);
        ck_assert_int_eq(round_up_power_of_two(511), 512);
        ck_assert_int_eq(round_up_power_of_two(32), 32);
}
END_TEST

START_TEST (test_histogram_simple)
{
        fill_rgba_pixels(&zero_fill);
        unsigned char * pixels = *image->pixels;
        for(int i = 0; i < width * height * RGB_CHANNEL; i+=4) {
                ck_assert_int_eq(pixels[i], 0);
                ck_assert_int_eq(pixels[i + 1], 0);
                ck_assert_int_eq(pixels[i + 2], 0);
        }

        init_job_from_image(image, job);
        generate_histogram(clinfo, image, job);
        clFinish(clinfo->command_queue);

        ck_assert_int_eq(job->result_size[0], 1);
        ck_assert_int_eq(job->result_size[1], 1);

        for(int c = 0; c < RGB_CHANNEL; c++) {
                assert_float_equals(job->results[c * BUCKET_NUMBER], 1.f);
                assert_float_equals(job->results[c * BUCKET_NUMBER + 1], 0.f);
                assert_float_equals(job->results[c * BUCKET_NUMBER + 2], 0.f);
                assert_float_equals(job->results[c * BUCKET_NUMBER + 3], 0.f);
                assert_float_equals(job->results[c * BUCKET_NUMBER + 4], 0.f);
        }
}
END_TEST

START_TEST (test_histogram_blue_green)
{
        fill_rgba_pixels(&blue_green_fill);
        init_job_from_image(image, job);
        generate_histogram(clinfo, image, job);
        clFinish(clinfo->command_queue);
        check_blue_green_results(job->results);
}
END_TEST

START_TEST (test_spilled_histogram)
{
        fill_rgba_pixels(&spilled_fill);

        init_job_from_image(image, job);
        generate_histogram(clinfo, image, job);
        clFinish(clinfo->command_queue);

        float * results = job->results;
        assert_float_equals(results[0 * BUCKET_NUMBER], 0.5f);
        assert_float_equals(results[0 * BUCKET_NUMBER + 4], 0.5f);
        assert_float_equals(results[1 * BUCKET_NUMBER + 1], 0.5f);
        assert_float_equals(results[1 * BUCKET_NUMBER + 4], 0.5f);
        assert_float_equals(results[2 * BUCKET_NUMBER + 2], 0.5f);
        assert_float_equals(results[2 * BUCKET_NUMBER + 3], 0.5f);
}
END_TEST

void create_test_file(char * path, unsigned char (*fill_funct)(int, int, int))
{
        width = 512;
        height = 512;
        image->path = path;
        image->size[0] = width;
        image->size[1] = height;
        unsigned char ** pixels = image->pixels;
        *pixels = malloc(sizeof(unsigned char) * image->size[0]
                        * image->size[1] * RGB_CHANNEL);
        for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                        int index = y * width * RGB_CHANNEL + x * RGB_CHANNEL;
                        (*pixels)[index] = (*fill_funct)(x, y, 0);
                        (*pixels)[index + 1] = (*fill_funct)(x, y, 1);
                        (*pixels)[index + 2] = (*fill_funct)(x, y, 2);
                }
        }
}

START_TEST (test_read_jpeg_from_file)
{
        char * path = "/tmp/test.jpg";
        list_t * files = NULL;
        files = list_append(files, path);
        create_test_file(path, blue_green_fill);
        write_jpeg_image(path, image);

        job_t * job = push_jobs(files, clinfo)->value;
        clFinish(clinfo->command_queue);
        check_blue_green_results(job->results);
}
END_TEST

START_TEST (test_read_png_from_file)
{
        char * path = "/tmp/test.png";
        list_t * files = NULL;
        files = list_append(files, path);
        create_test_file(path, blue_green_fill);
        write_png_image(path, image);

        job_t * job = push_jobs(files, clinfo)->value;
        clFinish(clinfo->command_queue);
        check_blue_green_results(job->results);
}
END_TEST

START_TEST (test_inegal_size)
{
        width = 2000;
        height = 3500;
        fill_rgba_pixels(&blue_green_fill);
        init_job_from_image(image, job);
        generate_histogram(clinfo, image, job);
        clFinish(clinfo->command_queue);
        float * results = job->results;
        for(int y = 0; y < job->result_size[1]; y++) {
                for(int x = 0; x < job->result_size[0]; x++) {
                        int index = y * job->result_size[0] * VECTOR_SIZE
                                + x * VECTOR_SIZE;
                        assert_float_equals(results[index + 0 * BUCKET_NUMBER], 1.f);
                        assert_float_equals(results[index + 1 * BUCKET_NUMBER + 1], 1.f);
                        assert_float_equals(results[index + 2 * BUCKET_NUMBER + 4], 1.f);
                }
        }
}
END_TEST

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

Suite * soragl_suite (void)
{
        Suite *s = suite_create ("cl_image");
        TCase *tc_core = tcase_create ("cl_image");
        tcase_add_checked_fixture (tc_core, setup, teardown);
        tcase_add_test (tc_core, test_near_power);
        tcase_add_test (tc_core, test_histogram_simple);
        tcase_add_test (tc_core, test_histogram_blue_green);
        tcase_add_test (tc_core, test_spilled_histogram);
        tcase_add_test (tc_core, test_inegal_size);
        tcase_add_test (tc_core, test_histogram_distance);
        tcase_add_test (tc_core, test_histogram_average);
        tcase_add_test (tc_core, test_read_jpeg_from_file);
        tcase_add_test (tc_core, test_read_png_from_file);
        suite_add_tcase (s, tc_core);
        return s;
}

int main (void)
{
        int number_failed;
        Suite *s = soragl_suite ();
        SRunner *sr = srunner_create (s);
        srunner_run_all (sr, CK_NORMAL);
        number_failed = srunner_ntests_failed (sr);
        srunner_free (sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
