#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <cl_image.h>
#include <image_utils.h>

void
setup (void)
{
}

void
teardown (void)
{
}

START_TEST (test_near_power)
{
  int counter = searchNearestPower(4);
  fail_unless(counter == 2, "Got %i", counter);
  counter = searchNearestPower(512);
  fail_unless(searchNearestPower(512) == 9, "Got %c", counter);
}
END_TEST

void fill_pixels(unsigned char (*fill_funct)(int, int, int), int height
    , int width, unsigned char ** pixels) {
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

unsigned char one_fill(int x, int y, int c) {
  return 1;
}

START_TEST (test_thumbnail)
{
  int width = 32;
  int height = 32;
  int resultWidth;
  int resultHeight;

  unsigned char ** results = malloc(sizeof(unsigned char **));
  unsigned char ** pixels = malloc(sizeof(unsigned char **));
  fill_pixels(&zero_fill, height, width, pixels);
  for(int i = 0; i < width * height * RGB_CHANNEL; i++) {
    fail_unless((*pixels)[i] == 0);
  }
  processImage(pixels, 32, 32, &resultWidth, &resultHeight, results);

  fail_unless(resultWidth == 1, "Result width : %d", resultWidth);
  fail_unless(resultHeight == 1, "Result height : %d", resultHeight);
  for(int c = 0; c < RGB_CHANNEL; c++) {
    fail_unless((*results)[c] == 0, "results[%i]==%u"
        , c, (*results)[c]);
  }
  fill_pixels(&one_fill, height, width, pixels);
}
END_TEST

Suite *
soragl_suite (void)
{
  Suite *s = suite_create ("cl_image");
  TCase *tc_core = tcase_create ("cl_image");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test (tc_core, test_near_power);
  tcase_add_test (tc_core, test_thumbnail);
  suite_add_tcase (s, tc_core);
  return s;
}

int
main (void)
{
  int number_failed;
  Suite *s = soragl_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
