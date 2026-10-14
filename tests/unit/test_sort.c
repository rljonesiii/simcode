/* Unit tests for sort.c: Sort(list, first, last) sorts list[first..last]. */
#include "minitest.h"
#include "sort.h"

static int is_sorted(const double *a, long first, long last)
{
  long i;
  for (i = first; i < last; i++) if (a[i] > a[i + 1]) return 0;
  return 1;
}

static void sorts_reversed_input(void)
{
  double a[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  Sort(a, 0, 9);
  CHECK(is_sorted(a, 0, 9));
  CHECK(a[0] == 0 && a[9] == 9);
}

static void keeps_duplicates(void)
{
  double a[] = {3, 1, 3, 2, 1, 3};
  double want[] = {1, 1, 2, 3, 3, 3};
  int    i, same = 1;
  Sort(a, 0, 5);
  for (i = 0; i < 6; i++) if (a[i] != want[i]) same = 0;
  CHECK(same);
}

static void single_element_and_sorted_input(void)
{
  double one[] = {42.0};
  double s[]   = {-1.5, 0.0, 2.25, 1e6};
  Sort(one, 0, 0);
  CHECK(one[0] == 42.0);
  Sort(s, 0, 3);
  CHECK(s[0] == -1.5 && s[1] == 0.0 && s[2] == 2.25 && s[3] == 1e6);
}

static void sorts_only_the_given_range(void)
{
  /* the simulators use 1-based arrays, e.g. Sort(x, 1, n) */
  double a[] = {100, 5, 4, 3, 2, 1, -100};
  Sort(a, 1, 5);
  CHECK(a[0] == 100 && a[6] == -100);
  CHECK(is_sorted(a, 1, 5));
  CHECK(a[1] == 1 && a[5] == 5);
}

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(sorts_reversed_input);
  RUN(keeps_duplicates);
  RUN(single_element_and_sorted_input);
  RUN(sorts_only_the_given_range);
  MT_MAIN_END("test_sort")
}
