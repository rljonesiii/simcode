/* Unit tests for rvms.c: pdf / cdf / idf functions. Reference values are
 * standard table values (normal, Student t, chi-square). */
#include "minitest.h"
#include "rvms.h"

static void log_factorial_and_choose(void)
{
  CHECK_NEAR(LogFactorial(0), 0.0, 1e-12);
  CHECK_NEAR(LogFactorial(5), log(120.0), 1e-10);
  CHECK_NEAR(LogChoose(5, 2), log(10.0), 1e-10);
  CHECK_NEAR(LogChoose(20, 10), log(184756.0), 1e-9);
}

static void normal_table_values(void)
{
  CHECK_NEAR(cdfNormal(0.0, 1.0, 0.0), 0.5, 1e-12);
  CHECK_NEAR(cdfNormal(0.0, 1.0, 1.959964), 0.975, 1e-6);
  CHECK_NEAR(cdfNormal(10.0, 2.0, 12.0), 0.8413447, 1e-6);
  CHECK_NEAR(idfNormal(0.0, 1.0, 0.975), 1.959964, 1e-5);
  CHECK_NEAR(idfNormal(0.0, 1.0, 0.5), 0.0, 1e-8);
  CHECK_NEAR(pdfNormal(0.0, 1.0, 0.0), 1.0 / sqrt(2.0 * 3.14159265358979), 1e-10);
}

static void student_table_values(void)
{
  /* used by the simulators for confidence intervals */
  CHECK_NEAR(idfStudent(10, 0.975), 2.228139, 1e-5);
  CHECK_NEAR(idfStudent(9, 0.995), 3.249836, 1e-5);
  CHECK_NEAR(idfStudent(999, 0.975), 1.962341, 1e-5);
  CHECK_NEAR(cdfStudent(5, 0.0), 0.5, 1e-10);
}

static void chisquare_table_values(void)
{
  CHECK_NEAR(cdfChisquare(1, 3.841459), 0.95, 1e-6);
  CHECK_NEAR(idfChisquare(2, 0.95), 5.991465, 1e-5);
  CHECK_NEAR(idfChisquare(10, 0.05), 3.940299, 1e-5);
}

static void exponential_closed_form(void)
{
  CHECK_NEAR(cdfExponential(2.0, 3.0), 1.0 - exp(-1.5), 1e-12);
  CHECK_NEAR(pdfExponential(2.0, 3.0), 0.5 * exp(-1.5), 1e-12);
  CHECK_NEAR(idfExponential(2.0, 1.0 - exp(-1.5)), 3.0, 1e-9);
}

static void continuous_idf_inverts_cdf(void)
{
  double us[] = {0.01, 0.1, 0.5, 0.9, 0.99};
  int    i;
  for (i = 0; i < 5; i++) {
    double u = us[i];
    CHECK_NEAR(cdfUniform(2.0, 5.0, idfUniform(2.0, 5.0, u)), u, 1e-9);
    CHECK_NEAR(cdfExponential(3.0, idfExponential(3.0, u)), u, 1e-9);
    CHECK_NEAR(cdfErlang(3, 2.0, idfErlang(3, 2.0, u)), u, 1e-7);
    CHECK_NEAR(cdfNormal(1.0, 2.0, idfNormal(1.0, 2.0, u)), u, 1e-7);
    CHECK_NEAR(cdfLognormal(0.0, 0.5, idfLognormal(0.0, 0.5, u)), u, 1e-7);
    CHECK_NEAR(cdfChisquare(4, idfChisquare(4, u)), u, 1e-7);
    CHECK_NEAR(cdfStudent(7, idfStudent(7, u)), u, 1e-7);
  }
}

static void discrete_pdfs_sum_to_one(void)
{
  long   x;
  double s_bin = 0.0, s_poi = 0.0, mean_poi = 0.0, s_geo = 0.0, s_pas = 0.0;
  for (x = 0; x <= 12; x++)  s_bin += pdfBinomial(12, 0.3, x);
  for (x = 0; x <= 80; x++) { s_poi += pdfPoisson(6.0, x); mean_poi += x * pdfPoisson(6.0, x); }
  for (x = 0; x <= 200; x++) s_geo += pdfGeometric(0.6, x);
  for (x = 0; x <= 300; x++) s_pas += pdfPascal(3, 0.5, x);
  CHECK_NEAR(s_bin, 1.0, 1e-10);
  CHECK_NEAR(s_poi, 1.0, 1e-10);
  CHECK_NEAR(mean_poi, 6.0, 1e-8);
  CHECK_NEAR(s_geo, 1.0, 1e-10);
  CHECK_NEAR(s_pas, 1.0, 1e-10);
  CHECK_NEAR(cdfBinomial(12, 0.3, 12), 1.0, 1e-12);
  CHECK_NEAR(pdfEquilikely(1, 6, 3), 1.0 / 6.0, 1e-12);
  CHECK_NEAR(cdfEquilikely(1, 6, 3), 0.5, 1e-12);
  CHECK_NEAR(pdfBernoulli(0.3, 1), 0.3, 1e-12);
  CHECK_NEAR(cdfGeometric(0.6, 2), 1.0 - pow(0.6, 3), 1e-12);
}

/* Park & Geyer discrete idf: smallest x with cdf(x) > u */
#define CHECK_DISCRETE_IDF(cdf_x, cdf_xm1, x) do {                           \
    CHECK((cdf_x) > u);                                                      \
    CHECK((x) == lo || (cdf_xm1) <= u);                                      \
  } while (0)

static void discrete_idf_inverts_cdf(void)
{
  double us[] = {0.01, 0.2, 0.5, 0.8, 0.99};
  int    i;
  long   x, lo = 0;
  for (i = 0; i < 5; i++) {
    double u = us[i];
    x = idfBinomial(12, 0.3, u);
    CHECK_DISCRETE_IDF(cdfBinomial(12, 0.3, x), cdfBinomial(12, 0.3, x - 1), x);
    x = idfPoisson(6.0, u);
    CHECK_DISCRETE_IDF(cdfPoisson(6.0, x), cdfPoisson(6.0, x - 1), x);
    x = idfGeometric(0.6, u);
    CHECK_DISCRETE_IDF(cdfGeometric(0.6, x), cdfGeometric(0.6, x - 1), x);
    x = idfPascal(3, 0.5, u);
    CHECK_DISCRETE_IDF(cdfPascal(3, 0.5, x), cdfPascal(3, 0.5, x - 1), x);
    lo = 1;
    x = idfEquilikely(1, 6, u);
    CHECK_DISCRETE_IDF(cdfEquilikely(1, 6, x), cdfEquilikely(1, 6, x - 1), x);
    lo = 0;
  }
}

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(log_factorial_and_choose);
  RUN(normal_table_values);
  RUN(student_table_values);
  RUN(chisquare_table_values);
  RUN(exponential_closed_form);
  RUN(continuous_idf_inverts_cdf);
  RUN(discrete_pdfs_sum_to_one);
  RUN(discrete_idf_inverts_cdf);
  MT_MAIN_END("test_rvms")
}
