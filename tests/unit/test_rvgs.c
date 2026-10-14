/* Unit tests for rvgs.c: sample moments and support of each random variate.
 * Seeds are fixed, so the tolerances (about 5 standard errors) are stable. */
#include "minitest.h"
#include "rngs.h"
#include "rvgs.h"

#define N 200000L

typedef double (*sampler)(void);

static double g_a, g_b, g_c;
static long   g_n;

static void moments(sampler f, double *mean, double *var)
{
  long   i;
  double x, sum = 0.0, sumsq = 0.0;
  PlantSeeds(123456789);
  SelectStream(0);
  for (i = 0; i < N; i++) { x = f(); sum += x; sumsq += x * x; }
  *mean = sum / N;
  *var  = sumsq / N - (*mean) * (*mean);
}

/* checks mean within 5 standard errors and variance within 3 percent */
#define CHECK_MOMENTS(f, mu, sigma2) do {                                    \
    double m_, v_;                                                           \
    moments(f, &m_, &v_);                                                    \
    CHECK_NEAR(m_, (mu), 5.0 * sqrt((sigma2) / N));                          \
    CHECK_NEAR(v_, (sigma2), 0.03 * (sigma2));                               \
  } while (0)

static double s_bernoulli(void)  { return (double)Bernoulli(g_a); }
static double s_binomial(void)   { return (double)Binomial(g_n, g_a); }
static double s_equilikely(void) { return (double)Equilikely((long)g_a, (long)g_b); }
static double s_geometric(void)  { return (double)Geometric(g_a); }
static double s_pascal(void)     { return (double)Pascal(g_n, g_a); }
static double s_poisson(void)    { return (double)Poisson(g_a); }
static double s_uniform(void)    { return Uniform(g_a, g_b); }
static double s_exponential(void){ return Exponential(g_a); }
static double s_erlang(void)     { return Erlang(g_n, g_a); }
static double s_normal(void)     { return Normal(g_a, g_b); }
static double s_lognormal(void)  { return Lognormal(g_a, g_b); }
static double s_chisquare(void)  { return Chisquare(g_n); }
static double s_student(void)    { return Student(g_n); }
static double s_triangle(void)   { return Triangle(g_a, g_b, g_c); }

static void bernoulli_moments(void)  { g_a = 0.3; CHECK_MOMENTS(s_bernoulli, 0.3, 0.21); }
static void binomial_moments(void)   { g_n = 10; g_a = 0.4; CHECK_MOMENTS(s_binomial, 4.0, 2.4); }
static void equilikely_moments(void) { g_a = 3; g_b = 8; CHECK_MOMENTS(s_equilikely, 5.5, 35.0 / 12.0); }
static void geometric_moments(void)  { g_a = 0.6; CHECK_MOMENTS(s_geometric, 1.5, 3.75); }
static void pascal_moments(void)     { g_n = 3; g_a = 0.5; CHECK_MOMENTS(s_pascal, 3.0, 6.0); }
static void poisson_moments(void)    { g_a = 4.0; CHECK_MOMENTS(s_poisson, 4.0, 4.0); }
static void uniform_moments(void)    { g_a = 2.0; g_b = 5.0; CHECK_MOMENTS(s_uniform, 3.5, 0.75); }
static void exponential_moments(void){ g_a = 2.0; CHECK_MOMENTS(s_exponential, 2.0, 4.0); }
static void erlang_moments(void)     { g_n = 3; g_a = 2.0; CHECK_MOMENTS(s_erlang, 6.0, 12.0); }
static void normal_moments(void)     { g_a = 1.0; g_b = 2.0; CHECK_MOMENTS(s_normal, 1.0, 4.0); }
static void chisquare_moments(void)  { g_n = 4; CHECK_MOMENTS(s_chisquare, 4.0, 8.0); }
static void student_moments(void)    { g_n = 10; CHECK_MOMENTS(s_student, 0.0, 10.0 / 8.0); }

static void lognormal_moments(void)
{
  g_a = 0.0; g_b = 0.5;
  CHECK_MOMENTS(s_lognormal, exp(0.125), (exp(0.25) - 1.0) * exp(0.25));
}

static void triangle_moments(void)
{
  /* Triangle(a, b, c): min a, max b, mode c */
  g_a = 1.0; g_b = 4.0; g_c = 2.0;
  CHECK_MOMENTS(s_triangle, 7.0 / 3.0, 7.0 / 18.0);
}

static void discrete_support(void)
{
  long i, x, bad = 0, saw_lo = 0, saw_hi = 0;
  PlantSeeds(1); SelectStream(0);
  for (i = 0; i < 20000; i++) {
    x = Equilikely(3, 8);
    if (x < 3 || x > 8) bad++;
    if (x == 3) saw_lo = 1;
    if (x == 8) saw_hi = 1;
    x = Bernoulli(0.5);
    if (x != 0 && x != 1) bad++;
    x = Binomial(5, 0.5);
    if (x < 0 || x > 5) bad++;
    if (Geometric(0.5) < 0 || Poisson(2.0) < 0 || Pascal(2, 0.5) < 0) bad++;
  }
  CHECK(bad == 0);
  CHECK(saw_lo && saw_hi);
}

static void continuous_support(void)
{
  long   i, bad = 0;
  double x;
  PlantSeeds(2); SelectStream(0);
  for (i = 0; i < 20000; i++) {
    x = Uniform(2.0, 5.0);       if (x <= 2.0 || x >= 5.0) bad++;
    x = Triangle(1.0, 4.0, 2.0); if (x < 1.0 || x > 4.0) bad++;
    if (Exponential(1.0) <= 0.0) bad++;
    if (Erlang(2, 1.0) <= 0.0) bad++;
    if (Lognormal(0.0, 1.0) <= 0.0) bad++;
    if (Chisquare(3) <= 0.0) bad++;
  }
  CHECK(bad == 0);
}

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(bernoulli_moments);
  RUN(binomial_moments);
  RUN(equilikely_moments);
  RUN(geometric_moments);
  RUN(pascal_moments);
  RUN(poisson_moments);
  RUN(uniform_moments);
  RUN(exponential_moments);
  RUN(erlang_moments);
  RUN(normal_moments);
  RUN(lognormal_moments);
  RUN(chisquare_moments);
  RUN(student_moments);
  RUN(triangle_moments);
  RUN(discrete_support);
  RUN(continuous_support);
  MT_MAIN_END("test_rvgs")
}
