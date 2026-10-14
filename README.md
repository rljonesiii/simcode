# simcode

Discrete-event simulation programs in ANSI C, written by Rob Jones for CSci 726 (Discrete-Event Simulation) in 1997 and revised through 2002. The centerpiece is **qnsim**, a general-purpose simulator for networks of single-server queues. Around it sit the course exercises and the Park & Geyer random number and statistics library they all share.

## Contents

| Area | What it is |
|---|---|
| `qnsim` (`qnet.c`, `qnsim*.c`, `queue.c`) | Queuing network simulator: describe a network in a small C file, get steady-state or transient estimates with confidence intervals |
| `rngs.c`, `rvgs.c`, `rvms.c` | Park & Geyer library: 256-stream Lehmer random number generator, random variate generators, and pdf/cdf/inverse-cdf functions |
| `ssq*.c`, `msq.c`, `mm1.c`, `tsim*.c`, `simjobs.c` | Single- and multi-server queue simulations |
| `sis*.c` | (s, S) inventory system simulations |
| `uvs.c`, `bvs.c`, `ddh.c`, `cdh.c`, `acs.c`, `estimate.c` | Data analysis filters: univariate and bivariate statistics, histograms, autocorrelation, interval estimates. They read one value per line from stdin. |
| `algC*.c`, `exC*.c`, `ex*.c`, `galileo.c`, `gamble*.c`, `buffon.c`, `monte*.c` | Course exercises: generator tests, Monte Carlo experiments |
| `qnsim/` | Distributable copy of the simulator, kept identical to the root sources |
| `tests/` | Unit and regression test suite |

## Requirements

- A C compiler (Apple clang or gcc) and `make`
- `bash` and `perl` for the test suite
- `csh`, only for the original `run` script

The code predates C99, so compile everything with `-std=gnu89 -w`. Modern compilers reject it otherwise.

## Quick start

Build and run the example network, the five-node network of example 9.5.4 in the course notes:

```bash
gcc -std=gnu89 -w rngs.c rvgs.c rvms.c queue.c misc.c qnet.c qnsim.c qnsim-ss.c example954.c -lm -o example954
./example954
```

```
Steady-State Simulation

  batch size.........:  1000
  number of batches..:  60
  confidence level...:  95%
  confidence interval:  +/- 5%

Expected number of jobs in the system:  14.3359 +/- 0.6757
Expected wait time of a job..........:  17.2721 +/- 0.8141
```

## qnsim, the queuing network simulator

### Describing a network

A model is a C file that defines two functions, `Network()` and `InitialState()`. Node 0 is the outside world: jobs arrive from it and depart to it. Service nodes are numbered from 1.

```c
#include "qnet.h"

double arrive(void)  { return Exponential(2.0); }   /* mean inter-arrival time 2.0 */
double service(void) { return Erlang(5, 2.0 / 7.0); }

void Network(void)
{
  Nodes(2);                      /* two service nodes, numbered 1 and 2      */
  Connect(0, 1, 1.0);            /* every arrival goes to node 1             */
  Connect(1, 2, 0.6);            /* node 1 routes to node 2 with prob 0.6    */
  Connect(1, 0, 0.4);            /*   ... or leaves the system with prob 0.4 */
  Connect(2, 0, 1.0);
  Arrival(arrive, 0.5);          /* function and mean arrival rate           */
  Service(1, service, 0.7);      /* function and mean service rate per node  */
  Service(2, service, 0.7);
}

void InitialState(void)
{
  /* empty network; or e.g. Initial(1, 3) to start node 1 with 3 jobs */
}
```

| Call | Purpose |
|---|---|
| `Nodes(n)` | Declare `n` service nodes. Must come first. |
| `Connect(i, j, p)` | Route a job leaving node `i` to node `j` with probability `p`. Each row should sum to 1. |
| `Arrival(f, rate)` | External inter-arrival time function and its mean rate. Makes the network open. |
| `Service(i, f, rate)` | Service time function and mean service rate for node `i`. |
| `Initial(i, j)` | Start node `i` with `j` jobs. Call it from `InitialState()`. |

Any generator from `rvgs.h` can be used inside the time functions, for example `Exponential`, `Erlang`, `Uniform`, `Normal` or `Triangle`. Node `i` draws from random number stream `i`, so changing one node's distribution does not disturb the others.

### Building a model

Link the model with the engine and one of the two drivers:

| Driver | Method | Reports |
|---|---|---|
| `qnsim-ss.c` | Steady state, batch means. Adds batches of 1000 departures until the 95% interval is within 5% of the estimate. | Mean number of jobs in the system and mean wait |
| `qnsim-ts.c` | Transient, 10,000 replications from the initial state | Mean number of external arrivals until flow in equals flow out |

Compile-time options:

| Flag | Effect |
|---|---|
| `-DSJF_QUEUE` | Shortest-job-first queues instead of FIFO. Service times are sampled on arrival. |
| `-DPLOT` | Transient driver prints per-replication data for plotting |
| `-DJACKSON` | Model-defined switch. `example954.c` uses it to swap in exponential service, which makes the network a Jackson network. |
| `-DDEBUG` | Debug output |

The original csh wrapper `run` does the same build: `run -h` lists its flags, and `run example954` builds and runs a model. It calls `gcc` without `-std=gnu89`, so it fails under current clang unless edited.

### Limitations

- Statistics are measured at node 0, so they are meaningful only for open networks. A closed network will not terminate.
- With FIFO queues, `Initial(i, j)` loads one job instead of `j`. See [Known issues](#known-issues).

## Other programs

Each is a single file linked against the library modules it includes:

```bash
gcc -std=gnu89 -w ssq3.c rngs.c -lm -o ssq3 && ./ssq3
gcc -std=gnu89 -w uvs.c -lm -o uvs && ./uvs < ddh.dat
```

Programs that call `PlantSeeds(0)` prompt for a seed on stdin.

## Tests

```bash
cd tests
make test
```

- **Unit tests** cover the random number generator, every variate generator's mean and variance, the distribution functions against table values, the sort routine, both queue types, and the network engine. They run under AddressSanitizer and UndefinedBehaviorSanitizer.
- **Regression tests** build 37 programs with fixed seeds and compare their output byte-for-byte against `tests/regress/golden/`. This covers every simulator mode and option.
- **Sync check** confirms that `qnsim/` matches the root sources.

`make golden` regenerates the expected outputs after an intended change. The golden files were produced on macOS arm64 with Apple clang. Other platforms may differ in the last digits. See `tests/Makefile` for the other targets.

## Known issues

- With FIFO queues, `Initial(i, j)` loads one job instead of `j`, because the FIFO `Insert` macro always adds one. The unit tests track this as a known bug.
- Several older files no longer build with modern compilers: `MMck.c`, `monte_carlo.c`, `multiproc.c`, `mobile-server.c`, `sssim.c` (C++), and `model1.c`/`model2.c` (written for an older `Arrival`/`Service` signature).
- `ssq1.c` and `sis1.c` need `.dat` input files that are not in the archive.

Two `qnet.c` bugs were fixed in 2026: an out-of-bounds read in `Init()`, and an infinite loop in `Initial()` with SJF queues.

## About the files

Compiled executables from the original SunOS (SPARC) and Cygwin builds are kept on disk but not tracked by git. Files such as `qnet.c.1`, `qnsim.c.2` and `makefile~`, plus the `.tar` and `.zip` archives, are historical snapshots and backups. They are not part of any build.

## Credits

- Random number, variate and statistics library (`rngs`, `rvgs`, `rvms`, `rng`) and several textbook programs by Steve Park and Dave Geyer.
- Generator from S. K. Park and K. W. Miller, "Random Number Generators: Good Ones Are Hard To Find", *Communications of the ACM*, October 1988.
- qnsim, the exercises, and the revisions noted in the source headers by Rob Jones.
