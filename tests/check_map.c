#include <stdlib.h>
#include <check.h>
#include <map.h>

void setup (void)
{
}

void teardown (void)
{
}

START_TEST (test_simple_map)
{
        map_t * map = map_create(4);
        map = map_add(map, "test", "test");
        map = map_add(map, "test2", "test2");
        char * res = (char *) map_get(map, "test");
        ck_assert_str_eq(res, "test");
        res = (char *) map_get(map, "test2");
        ck_assert_str_eq(res, "test2");
        map_release(map);
}
END_TEST

START_TEST (test_collision)
{
        map_t * map = map_create(1);
        map = map_add(map, "test", "test");
        map = map_add(map, "test2", "test2");
        char * res = (char *) map_get(map, "test");
        ck_assert_str_eq(res, "test");
        res = (char *) map_get(map, "test2");
        ck_assert_str_eq(res, "test2");
        map_release(map);
}
END_TEST

START_TEST (test_deletion)
{
        map_t * map = map_create(1);
        map = map_add(map, "test", "test");
        map = map_add(map, "test2", "test2");
        map_delete(map, "test");
        char * res = (char *) map_get(map, "test");
        ck_assert_str_eq(res, "test");
        map_release(map);
}
END_TEST

Suite * map_suite (void)
{
        Suite *s = suite_create ("map");
        TCase *tc_core = tcase_create ("map");
        tcase_add_checked_fixture (tc_core, setup, teardown);
        tcase_add_test (tc_core, test_simple_map);
        tcase_add_test (tc_core, test_collision);
        tcase_add_test (tc_core, test_deletion);
        suite_add_tcase (s, tc_core);
        return s;
}

int main (void)
{
        int number_failed;
        Suite *s = map_suite();
        SRunner *sr = srunner_create (s);
        srunner_run_all (sr, CK_NORMAL);
        number_failed = srunner_ntests_failed (sr);
        srunner_free (sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
