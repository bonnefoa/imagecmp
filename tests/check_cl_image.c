#include <stdlib.h>
#include <check.h>
#include <cl_image.h>

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
  fail_unless(searchNearestPower(512) == 9);
}
END_TEST

Suite *
soragl_suite (void)
{
  Suite *s = suite_create ("cl_image");
  TCase *tc_core = tcase_create ("cl_image");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test (tc_core, test_near_power);
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
