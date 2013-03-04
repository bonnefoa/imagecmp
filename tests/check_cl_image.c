#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <cl_image.h>
#include <cl_histogram.h>
#include <image_utils.h>
#include <image_utils.h>

#define assertFloatEquals(res, expected) fail_unless(res == expected, "Expected %f, got %f", expected, res)

int width = 32;
int height = 32;
int resultWidth;
int resultHeight;
float ** results;
unsigned char ** pixels;
cl_struct clStruct;

void setup (void) {
  results = malloc(sizeof(float **));
  pixels = malloc(sizeof(unsigned char **));
  clStruct = initCl("src/kernel_image.cl", "generateHistogram");
}

void teardown (void) {
  cleanCl(clStruct);
  free(results);
  free(pixels);
}

void fill_rgba_pixels(unsigned char (*fill_funct)(int, int, int)
    , int width, int height, unsigned char ** pixels) {
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
}

unsigned char zero_fill(int x, int y, int c) {
  return 0;
}

unsigned char blue_green_fill(int x, int y, int c) {
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

unsigned char spilled_fill(int x, int y, int c) {
  switch (c) {
    case 0 :
      if (x < 16)
        return 0;
      return 254;
    case 1 :
      if (x < 16)
        return 51;
      return 204;
    case 2 :
      if (x < 16)
        return 102;
      return 153;
  }
  return 0;
}

void check_blue_green_results(float ** results) {
  assertFloatEquals((*results)[0 * BUCKET_NUMBER], 1.f);
  assertFloatEquals((*results)[1 * BUCKET_NUMBER + 1], 1.f);
  assertFloatEquals((*results)[2 * BUCKET_NUMBER + 4], 1.f);
}

START_TEST (test_near_power) {
  ck_assert_int_eq(roundUpPowerOfTwo(3), 4);
  ck_assert_int_eq(roundUpPowerOfTwo(511), 512);
  ck_assert_int_eq(roundUpPowerOfTwo(32), 32);
}
END_TEST

START_TEST (test_histogram_simple) {
  fill_rgba_pixels(&zero_fill, width, height, pixels);
  for(int i = 0; i < width * height * RGB_CHANNEL; i+=4) {
    ck_assert_int_eq((*pixels)[i], 0);
    ck_assert_int_eq((*pixels)[i + 1], 0);
    ck_assert_int_eq((*pixels)[i + 2], 0);
  }

  generateHistogram(clStruct, pixels, width, height
      , &resultWidth, &resultHeight, results);

  ck_assert_int_eq(resultWidth, 1);
  ck_assert_int_eq(resultHeight, 1);

  for(int c = 0; c < RGB_CHANNEL; c++) {
    assertFloatEquals((*results)[c * BUCKET_NUMBER], 1.f);
    assertFloatEquals((*results)[c * BUCKET_NUMBER + 1], 0.f);
    assertFloatEquals((*results)[c * BUCKET_NUMBER + 2], 0.f);
    assertFloatEquals((*results)[c * BUCKET_NUMBER + 3], 0.f);
    assertFloatEquals((*results)[c * BUCKET_NUMBER + 4], 0.f);
  }
}
END_TEST

START_TEST (test_histogram_blue_green) {
  fill_rgba_pixels(&blue_green_fill, width, height, pixels);

  generateHistogram(clStruct, pixels, width, height
      , &resultWidth, &resultHeight, results);

  check_blue_green_results(results);
}
END_TEST

START_TEST (test_spilled_histogram) {
  fill_rgba_pixels(&spilled_fill, width, height, pixels);

  generateHistogram(clStruct, pixels, width, height
      , &resultWidth, &resultHeight, results);

  assertFloatEquals((*results)[0 * BUCKET_NUMBER], 0.5f);
  assertFloatEquals((*results)[0 * BUCKET_NUMBER + 4], 0.5f);
  assertFloatEquals((*results)[1 * BUCKET_NUMBER + 1], 0.5f);
  assertFloatEquals((*results)[1 * BUCKET_NUMBER + 4], 0.5f);
  assertFloatEquals((*results)[2 * BUCKET_NUMBER + 2], 0.5f);
  assertFloatEquals((*results)[2 * BUCKET_NUMBER + 3], 0.5f);
}
END_TEST

START_TEST (test_read_from_file) {
  width = 512;
  height = 512;
  int resultWidth, resultHeight;
  char * imageSource = "/tmp/test.jpg";
  *pixels = malloc(sizeof(unsigned char) * width * height * RGB_CHANNEL);
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      int index = y * width * RGB_CHANNEL + x * RGB_CHANNEL;
      (*pixels)[index] = (*blue_green_fill)(x, y, 0);
      (*pixels)[index + 1] = (*blue_green_fill)(x, y, 1);
      (*pixels)[index + 2] = (*blue_green_fill)(x, y, 2);
    }
  }
  writeJpegImage(imageSource, *pixels, width, height);
  generateHistogramFromFile(clStruct, imageSource
      , &resultWidth, &resultHeight, results);

  check_blue_green_results(results);
}
END_TEST

START_TEST (test_inegal_size) {
  width = 60;
  height = 50;
  fill_rgba_pixels(&blue_green_fill, width, height, pixels);
  generateHistogram(clStruct, pixels
      , width, height
      , &resultWidth, &resultHeight, results);

  for(int y = 0; y < resultWidth; y++) {
    for(int x = 0; x < resultWidth; x++) {
      assertFloatEquals((*results)[y * x * 16 + 0 * BUCKET_NUMBER], 1.f);
      assertFloatEquals((*results)[y * x * 16 + 1 * BUCKET_NUMBER + 1], 1.f);
      assertFloatEquals((*results)[y * x * 16 + 2 * BUCKET_NUMBER + 4], 1.f);
    }
  }

}
END_TEST

Suite * soragl_suite (void) {
  Suite *s = suite_create ("cl_image");
  TCase *tc_core = tcase_create ("cl_image");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test (tc_core, test_near_power);
  tcase_add_test (tc_core, test_histogram_simple);
  tcase_add_test (tc_core, test_histogram_blue_green);
  tcase_add_test (tc_core, test_spilled_histogram);
  tcase_add_test (tc_core, test_read_from_file);
  tcase_add_test (tc_core, test_inegal_size);
  suite_add_tcase (s, tc_core);
  return s;
}

int main (void) {
  int number_failed;
  Suite *s = soragl_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
