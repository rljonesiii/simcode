/******************************************************************************
*  minitest.h -- a tiny, dependency-free unit test harness for simcode.        *
*                                                                             *
*  CHECK(cond)                 fails if cond is false                         *
*  CHECK_NEAR(a, e, tol)       fails if |a - e| > tol                         *
*  CHECK_XFAIL(cond, why)      known bug: reported, not counted as a failure  *
*                              while cond is false; an unexpected pass IS a   *
*                              failure, so the XFAIL gets removed once fixed  *
*  RUN(fn)                     runs test fn (skipped if argv[1] filter given  *
*                              and is not a substring of the test name)       *
*  mt_isolate(fn, secs, &res)  runs fn in a forked child with a timeout,      *
*                              capturing stdout+stderr; for code that calls   *
*                              exit() via error() or that might hang          *
*  MT_MAIN_BEGIN / MT_MAIN_END wrap main()                                    *
*******************************************************************************/
#ifndef MINITEST_H
#define MINITEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int         mt_checks   = 0;
static int         mt_failures = 0;
static int         mt_xfails   = 0;
static const char *mt_current  = "";
static const char *mt_filter   = NULL;

#define MT_FAIL_HEADER() printf("  FAIL %s:%d [%s] ", __FILE__, __LINE__, mt_current)

#define CHECK(cond) do {                                                     \
    mt_checks++;                                                             \
    if (!(cond)) { mt_failures++; MT_FAIL_HEADER(); printf("%s\n", #cond); } \
  } while (0)

#define CHECK_NEAR(actual, expected, tol) do {                               \
    double mt_a_ = (double)(actual), mt_e_ = (double)(expected);             \
    mt_checks++;                                                             \
    if (!(fabs(mt_a_ - mt_e_) <= (double)(tol))) {                           \
      mt_failures++; MT_FAIL_HEADER();                                       \
      printf("%s = %.12g, expected %.12g +/- %g\n",                          \
             #actual, mt_a_, mt_e_, (double)(tol));                          \
    }                                                                        \
  } while (0)

#define CHECK_XFAIL(cond, why) do {                                          \
    mt_checks++;                                                             \
    if (cond) {                                                              \
      mt_failures++; MT_FAIL_HEADER();                                       \
      printf("XPASS %s -- known bug appears fixed, change to CHECK\n", #cond);\
    } else {                                                                 \
      mt_xfails++;                                                           \
      printf("  XFAIL %s:%d [%s] %s (known bug: %s)\n",                      \
             __FILE__, __LINE__, mt_current, #cond, why);                    \
    }                                                                        \
  } while (0)

#define RUN(fn) do {                                                         \
    if (mt_filter == NULL || strstr(#fn, mt_filter) != NULL) {               \
      mt_current = #fn; fn();                                                \
    }                                                                        \
  } while (0)

#define MT_MAIN_BEGIN(argc, argv) \
  if ((argc) > 1) mt_filter = (argv)[1];

#define MT_MAIN_END(name)                                                    \
  printf("%-22s %4d checks, %d failed, %d known bugs\n",                     \
         name, mt_checks, mt_failures, mt_xfails);                           \
  return (mt_failures == 0) ? 0 : 1;

/* ---------------------------------------------------------------------------
 * Process isolation.  Child exit codes:
 *   MT_CHILD_RETURNED  fn returned normally with no failed CHECKs
 *   MT_CHILD_FAILED    fn returned but a CHECK inside the child failed
 *   anything else      fn called exit() itself (simcode's error() exits 1)
 * ------------------------------------------------------------------------ */
#define MT_CHILD_RETURNED 101
#define MT_CHILD_FAILED   102

typedef struct {
  int  exited;       /* child exited (did not die from a signal)      */
  int  exit_code;    /* valid when exited                             */
  int  timed_out;    /* child was killed by the alarm                 */
  int  signal;       /* signal number when !exited                    */
  char out[8192];    /* captured output (truncated, NUL terminated)   */
} mt_result;

[[maybe_unused]] static void mt_isolate(void (*fn)(void), unsigned seconds, mt_result *r)
{
  int     fd[2];
  pid_t   pid;
  int     status;
  size_t  len = 0;
  ssize_t n;

  memset(r, 0, sizeof(*r));
  fflush(stdout);
  if (pipe(fd) != 0) { perror("pipe"); exit(2); }
  pid = fork();
  if (pid < 0) { perror("fork"); exit(2); }
  if (pid == 0) {                       /* child */
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);         /* capture stdout and stderr */
    dup2(fd[1], STDERR_FILENO);
    close(fd[1]);
    signal(SIGALRM, SIG_DFL);
    alarm(seconds);
    mt_failures = 0;
    fn();
    fflush(stdout);
    _exit(mt_failures ? MT_CHILD_FAILED : MT_CHILD_RETURNED);
  }
  close(fd[1]);                         /* parent: drain pipe, then reap */
  while ((n = read(fd[0], r->out + len, sizeof(r->out) - 1 - len)) > 0) {
    len += (size_t)n;
    if (len >= sizeof(r->out) - 1) {    /* buffer full: discard the rest */
      char sink[512];
      while (read(fd[0], sink, sizeof(sink)) > 0) {}
      break;
    }
  }
  r->out[len] = '\0';
  close(fd[0]);
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    r->exited    = 1;
    r->exit_code = WEXITSTATUS(status);
  } else if (WIFSIGNALED(status)) {
    r->signal    = WTERMSIG(status);
    r->timed_out = (r->signal == SIGALRM);
  }
}

/* Check that an isolated child returned cleanly; echo its output if not. */
#define CHECK_CHILD_OK(res) do {                                             \
    mt_checks++;                                                             \
    if (!((res).exited && (res).exit_code == MT_CHILD_RETURNED)) {           \
      mt_failures++; MT_FAIL_HEADER();                                       \
      if ((res).timed_out) printf("child timed out (infinite loop?)\n");     \
      else if (!(res).exited) printf("child killed by signal %d\n",          \
                                     (res).signal);                          \
      else printf("child exit code %d\n", (res).exit_code);                  \
      printf("%s", (res).out);                                               \
    }                                                                        \
  } while (0)

#endif /* MINITEST_H */
