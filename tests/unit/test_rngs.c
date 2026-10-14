/* Unit tests for rngs.c: the Park & Geyer multi-stream Lehmer generator. */
#include "minitest.h"
#include "rngs.h"

#define MODULUS 2147483647L

static void park_miller_reference_value(void)
{
  /* the library's own correctness check: seed 1, 10000 draws */
  long i, x;
  SelectStream(0);
  PutSeed(1);
  for (i = 0; i < 10000; i++) Random();
  GetSeed(&x);
  CHECK(x == 399268537L);
}

static void first_draw_from_seed_one(void)
{
  SelectStream(0);
  PutSeed(1);
  CHECK_NEAR(Random(), 48271.0 / MODULUS, 1e-15);
}

static void plant_seeds_uses_jump_multiplier(void)
{
  long x;
  SelectStream(1);
  PlantSeeds(1);
  GetSeed(&x);
  CHECK(x == 22925L);
  SelectStream(0);
  GetSeed(&x);
  CHECK(x == 1L);
}

static void draws_strictly_inside_unit_interval(void)
{
  long   i, bad = 0;
  double u;
  PlantSeeds(987654321);
  SelectStream(0);
  for (i = 0; i < 200000; i++) {
    u = Random();
    if (!(u > 0.0 && u < 1.0)) bad++;
  }
  CHECK(bad == 0);
}

static void uniform_moments(void)
{
  long   i, n = 200000;
  double u, sum = 0.0, sumsq = 0.0, mean, var;
  PlantSeeds(123456789);
  SelectStream(7);
  for (i = 0; i < n; i++) { u = Random(); sum += u; sumsq += u * u; }
  mean = sum / n;
  var  = sumsq / n - mean * mean;
  CHECK_NEAR(mean, 0.5, 5.0 * sqrt(1.0 / 12.0 / n));
  CHECK_NEAR(var, 1.0 / 12.0, 0.002);
}

static void streams_are_independent(void)
{
  double a1, b1, a2, b2;
  int    i;
  PlantSeeds(4242);
  SelectStream(3); a1 = Random();
  SelectStream(4); for (i = 0; i < 5; i++) Random();   /* disturb another stream */
  SelectStream(3); b1 = Random();

  PlantSeeds(4242);
  SelectStream(3); a2 = Random(); b2 = Random();
  CHECK(a1 == a2);
  CHECK(b1 == b2);
}

static void same_seed_same_sequence(void)
{
  double s1[50], s2[50];
  int    i, same = 1;
  PlantSeeds(55555); SelectStream(12);
  for (i = 0; i < 50; i++) s1[i] = Random();
  PlantSeeds(55555); SelectStream(12);
  for (i = 0; i < 50; i++) s2[i] = Random();
  for (i = 0; i < 50; i++) if (s1[i] != s2[i]) same = 0;
  CHECK(same);
}

static void put_seed_reduces_modulo(void)
{
  long x;
  SelectStream(0);
  PutSeed(MODULUS + 5);
  GetSeed(&x);
  CHECK(x == 5);
}

static void select_stream_wraps_at_256(void)
{
  long x, y;
  PlantSeeds(1);
  SelectStream(2);       GetSeed(&x);
  SelectStream(256 + 2); GetSeed(&y);
  CHECK(x == y);
}

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(park_miller_reference_value);
  RUN(first_draw_from_seed_one);
  RUN(plant_seeds_uses_jump_multiplier);
  RUN(draws_strictly_inside_unit_interval);
  RUN(uniform_moments);
  RUN(streams_are_independent);
  RUN(same_seed_same_sequence);
  RUN(put_seed_reduces_modulo);
  RUN(select_stream_wraps_at_256);
  MT_MAIN_END("test_rngs")
}
