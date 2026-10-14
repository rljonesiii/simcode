/* Unit tests for qnet.c (network construction) and qnsim.c (arrival and
 * service event helpers).  Built twice: default FIFO and -DSJF_QUEUE.
 *
 * qnet.c keeps its state in globals and reports user errors through
 * error(), which calls exit().  Every test therefore runs in a forked child
 * (mt_isolate) so that tests cannot leak state into each other, exits can be
 * observed, and an infinite loop shows up as a timeout instead of a hang. */
#include "minitest.h"
#include "qnet.h"
#include "qnsim.h"
#include "queue.h"
#include "misc.h"

extern qnt         type;
extern int         num_nodes;
extern queue_type *queue;
extern actvty     *status;
extern double    **P;
extern double     *rates;
extern rv_func    *random_time;

/* ---- model hooks required by qnet.c ------------------------------------ */

static void (*initial_hook)(void) = NULL;

void InitialState(void)
{
  if (initial_hook != NULL) initial_hook();
}

/* deterministic "random" times: returns seq[0], seq[1], ... in order */
static double seq[16];
static int    seq_len = 0, seq_pos = 0;

static void set_seq(const double *v, int n)
{
  int i;
  for (i = 0; i < n; i++) seq[i] = v[i];
  seq_len = n;
  seq_pos = 0;
}

static double next_in_seq(void) { return seq[seq_pos++ % seq_len]; }
static double constant_one(void) { return 1.0; }

static void build_three_node_network(void)
{
  Nodes(3);
  Connect(0, 1, 1.0);
  Connect(1, 2, 0.25);
  Connect(1, 3, 0.75);
  Connect(2, 0, 1.0);
  Connect(3, 0, 0.5);
  Connect(3, 1, 0.5);
  Arrival(constant_one, 2.0);
  Service(1, constant_one, 3.0);
  Service(2, constant_one, 4.0);
  Service(3, constant_one, 5.0);
}

/* ---- child bodies ------------------------------------------------------ */

static void c_nodes_allocates_external_plus_n(void)
{
  int i, j, clean = 1;
  Nodes(3);
  CHECK(num_nodes == 4);
  for (i = 0; i < num_nodes; i++) {
    if (random_time[i].func != NULL) clean = 0;
    if (status[i] != IDLE) clean = 0;
    if (rates[i] != 0.0) clean = 0;
    for (j = 0; j < num_nodes; j++) if (P[i][j] != 0.0) clean = 0;
  }
  for (i = 1; i < num_nodes; i++) if (Number(&queue[i]) != 0) clean = 0;
  CHECK(clean);
}

static void c_full_network_passes_init(void)
{
  /* regression: Init() used to read random_time[num_nodes], one past the
   * end, and could reject a correctly specified network */
  build_three_node_network();
  Init();
  CHECK(type == OPEN);
}

static void c_init_makes_rows_cumulative(void)
{
  build_three_node_network();
  Init();
  CHECK_NEAR(P[1][1], 0.0, 1e-12);
  CHECK_NEAR(P[1][2], 0.25, 1e-12);
  CHECK_NEAR(P[1][3], 1.0, 1e-12);
  CHECK_NEAR(P[3][0], 0.5, 1e-12);
  CHECK_NEAR(P[3][1], 1.0, 1e-12);
  CHECK_NEAR(P[0][3], 1.0, 1e-12);
}

static void c_mean_rates_recorded(void)
{
  build_three_node_network();
  CHECK(GetMeanRate(0) == 2.0);
  CHECK(GetMeanRate(1) == 3.0);
  CHECK(GetMeanRate(3) == 5.0);
}

static void c_routing_follows_probabilities(void)
{
  long i, n = 100000, to2 = 0, to3 = 0, other = 0, from2_to0 = 0;
  build_three_node_network();
  Init();
  PlantSeeds(12345);
  for (i = 0; i < n; i++) {
    int j = GetNextNode(1);
    if (j == 2) to2++; else if (j == 3) to3++; else other++;
    if (GetNextNode(2) == 0) from2_to0++;
  }
  CHECK(other == 0);
  CHECK(from2_to0 == n);
  CHECK_NEAR((double)to2 / n, 0.25, 0.01);
  CHECK_NEAR((double)to3 / n, 0.75, 0.01);
}

static void c_init_without_network(void)     { Init(); }
static void c_connect_before_nodes(void)     { Connect(0, 1, 1.0); }
static void c_service_node_zero(void)        { Nodes(1); Service(0, constant_one, 1.0); }
static void c_missing_service_function(void)
{
  Nodes(2);
  Arrival(constant_one, 1.0);
  Service(1, constant_one, 1.0);   /* node 2 never gets one */
  Init();
}
static void c_open_network_null_arrival(void)
{
  Nodes(1);
  Arrival(NULL, 1.0);
  Service(1, constant_one, 1.0);
  Init();
}

static void c_initial_loads_jobs(void)
{
  double times[] = {5.0, 1.0, 3.0};
  Nodes(2);
  Service(1, next_in_seq, 1.0);
  Service(2, constant_one, 1.0);
  set_seq(times, 3);
  Initial(1, 3);
  printf("count=%ld\n", (long)Number(&queue[1]));
#ifdef SJF_QUEUE
  {
    item_type a, b, c;
    Remove(&queue[1], &a); Remove(&queue[1], &b); Remove(&queue[1], &c);
    CHECK(a == 1.0 && b == 3.0 && c == 5.0);   /* stored sorted, SJF */
  }
#endif
}

static void initial_hook_node2_four_jobs(void) { Initial(2, 4); }

static void c_initialize_resets_and_reapplies(void)
{
  Nodes(2);
  Service(1, constant_one, 1.0);
  Service(2, constant_one, 1.0);
  Insert(&queue[1], 1.0);
  Insert(&queue[1], 1.0);
  initial_hook = initial_hook_node2_four_jobs;
  Initialize();
  CHECK(Number(&queue[1]) == 0);
  printf("count=%ld\n", (long)Number(&queue[2]));
}

/* qnsim.c event helpers */
static void c_arrival_to_idle_node_starts_service(void)
{
  double fe[2] = {0.0, 0.0};
  double times[] = {2.5};
  Nodes(1);
  Service(1, next_in_seq, 1.0);
  set_seq(times, 1);
  ProcessArrival(1, 10.0, fe);
  CHECK(status[1] == BUSY);
  CHECK_NEAR(fe[1], 12.5, 1e-12);
  CHECK(Number(&queue[1]) == 0);
}

static void c_arrival_to_busy_node_queues(void)
{
  double fe[2] = {0.0, 0.0};
  double times[] = {2.5, 7.0, 1.0};
  Nodes(1);
  Service(1, next_in_seq, 1.0);
  set_seq(times, 3);
  ProcessArrival(1, 10.0, fe);          /* starts service, uses 2.5      */
  ProcessArrival(1, 11.0, fe);          /* queued                        */
  ProcessArrival(1, 11.5, fe);          /* queued                        */
  CHECK(Number(&queue[1]) == 2);
  CHECK_NEAR(fe[1], 12.5, 1e-12);       /* departure time not disturbed  */
}

static void c_service_job_takes_next_job(void)
{
  double fe[2] = {0.0, 0.0};
  double times[] = {2.5, 7.0, 1.0};
  Nodes(1);
  Service(1, next_in_seq, 1.0);
  set_seq(times, 3);
  ProcessArrival(1, 10.0, fe);
  ProcessArrival(1, 11.0, fe);
  ProcessArrival(1, 11.5, fe);
  ServiceJob(1, 12.5, fe);
  CHECK(Number(&queue[1]) == 1);
  CHECK(status[1] == BUSY);
#ifdef SJF_QUEUE
  /* SJF samples at arrival (7.0 then 1.0) and serves the shortest: 1.0 */
  CHECK_NEAR(fe[1], 13.5, 1e-12);
#else
  /* FIFO samples at service start: next value in the sequence is 7.0 */
  CHECK_NEAR(fe[1], 19.5, 1e-12);
#endif
}

/* ---- tests (parent side) ----------------------------------------------- */

#define ISOLATED_OK(child) do {                                              \
    mt_result r_;                                                            \
    mt_isolate(child, 10, &r_);                                              \
    CHECK_CHILD_OK(r_);                                                      \
  } while (0)

/* child must exit through error() and print a message containing 'msg' */
#define ISOLATED_ERROR(child, msg) do {                                      \
    mt_result r_;                                                            \
    mt_isolate(child, 10, &r_);                                              \
    CHECK(r_.exited && r_.exit_code != MT_CHILD_RETURNED                     \
          && r_.exit_code != MT_CHILD_FAILED);                               \
    CHECK(strstr(r_.out, msg) != NULL);                                      \
  } while (0)

static void nodes_allocates_external_plus_n(void) { ISOLATED_OK(c_nodes_allocates_external_plus_n); }
static void full_network_passes_init(void)        { ISOLATED_OK(c_full_network_passes_init); }
static void init_makes_rows_cumulative(void)      { ISOLATED_OK(c_init_makes_rows_cumulative); }
static void mean_rates_recorded(void)             { ISOLATED_OK(c_mean_rates_recorded); }
static void routing_follows_probabilities(void)   { ISOLATED_OK(c_routing_follows_probabilities); }

static void user_errors_are_reported(void)
{
  ISOLATED_ERROR(c_init_without_network, "Must specify queuing network");
  ISOLATED_ERROR(c_connect_before_nodes, "Must first define the number of nodes");
  ISOLATED_ERROR(c_service_node_zero, "greater than zero");
  ISOLATED_ERROR(c_missing_service_function, "service-time function for every");
  ISOLATED_ERROR(c_open_network_null_arrival, "arrival-time function");
}

static void initial_loads_jobs(void)
{
  /* regression (SJF): Initial() used to loop forever (j++ instead of k++) */
  mt_result r;
  mt_isolate(c_initial_loads_jobs, 10, &r);
  CHECK_CHILD_OK(r);
#ifdef SJF_QUEUE
  CHECK(strstr(r.out, "count=3") != NULL);
#else
  CHECK_XFAIL(strstr(r.out, "count=3") != NULL,
              "FIFO Insert macro adds one job, so Initial(i, j) loads 1 not j");
#endif
}

static void initialize_resets_and_reapplies(void)
{
  mt_result r;
  mt_isolate(c_initialize_resets_and_reapplies, 10, &r);
  CHECK_CHILD_OK(r);
#ifdef SJF_QUEUE
  CHECK(strstr(r.out, "count=4") != NULL);
#else
  CHECK_XFAIL(strstr(r.out, "count=4") != NULL,
              "FIFO Insert macro adds one job, so Initial(i, j) loads 1 not j");
#endif
}

static void arrival_to_idle_node_starts_service(void) { ISOLATED_OK(c_arrival_to_idle_node_starts_service); }
static void arrival_to_busy_node_queues(void)         { ISOLATED_OK(c_arrival_to_busy_node_queues); }
static void service_job_takes_next_job(void)          { ISOLATED_OK(c_service_job_takes_next_job); }

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(nodes_allocates_external_plus_n);
  RUN(full_network_passes_init);
  RUN(init_makes_rows_cumulative);
  RUN(mean_rates_recorded);
  RUN(routing_follows_probabilities);
  RUN(user_errors_are_reported);
  RUN(initial_loads_jobs);
  RUN(initialize_resets_and_reapplies);
  RUN(arrival_to_idle_node_starts_service);
  RUN(arrival_to_busy_node_queues);
  RUN(service_job_takes_next_job);
#ifdef SJF_QUEUE
  MT_MAIN_END("test_qnet (SJF)")
#else
  MT_MAIN_END("test_qnet (FIFO)")
#endif
}
