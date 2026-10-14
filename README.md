# simcode

Discrete-event simulation programs in C, written by Rob Jones for CSci 726 (Discrete-Event Simulation) in 1997 and revised through 2002. The centerpiece is **qnsim**, a general-purpose simulator for networks of single-server queues. Around it sit the course exercises and the Park & Geyer random number and statistics library they all share.

In 2026 the code was brought up to **C23** with a CMake build, clang-tidy, sanitizers and a regression suite. Every program that ran before produces byte-identical output.

## Contents

| Area | What it is |
|---|---|
| `qnsim` (`qnet.c`, `qnsim*.c`, `queue.c`) | Queuing network simulator: describe a network in a small C file, get steady-state or transient estimates with confidence intervals |
| `rngs.c`, `rvgs.c`, `rvms.c` | Park & Geyer library: 256-stream Lehmer random number generator, random variate generators, and pdf/cdf/inverse-cdf functions |
| `ssq*.c`, `msq.c`, `mm1.c`, `sssim.c`, `tsim*.c`, `simjobs.c`, `mobile-server.c` | Single- and multi-server queue simulations |
| `sis*.c` | (s, S) inventory system simulations |
| `uvs.c`, `bvs.c`, `ddh.c`, `cdh.c`, `cdhcl.c`, `acs.c`, `estimate.c` | Data analysis filters: statistics, histograms, autocorrelation and interval estimates for data read from stdin, one value per line |
| `algC*.c`, `exC*.c`, `ex*.c`, `MMck.c`, `galileo.c`, `gamble*.c`, `buffon.c`, `monte*.c`, `urn*.c` | Course exercises: generator tests, Monte Carlo experiments |
| `qnsim/` | Distributable copy of the simulator, kept identical to the root sources |
| `tests/` | Unit and regression test suite |

## Requirements

- A C23 compiler: Clang 18 or later, or GCC 14 or later. Apple clang 21 works.
- CMake 3.28 or later
- `bash` and `perl` for the regression tests
- Optional: Homebrew LLVM (`brew install llvm`) for `clang-tidy` and a current `clang-format`
- Optional: `csh`, only for the original `run` script

## Quick start

```bash
make
```

This runs the `dev` CMake workflow: a debug build with AddressSanitizer and UndefinedBehaviorSanitizer, followed by the full test suite. Executables land in `build/dev/bin/`. Run the example network, the five-node network of example 9.5.4 in the course notes:

```bash
build/dev/bin/example954
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

Other entry points:

| Command | Does |
|---|---|
| `make release` | Optimized build and tests, in `build/release/` |
| `make lint` | Compiles everything through clang-tidy; any finding fails the build |
| `make golden` | Regenerates the regression golden files |
| `make clean` | Deletes all build trees |

The `make` targets wrap CMake presets, so `cmake --workflow --preset dev` and friends work directly too. Every build uses `-std=c23 -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Werror`.

## qnsim, the queuing network simulator

### Describing a network

A model is a C file that defines two functions, `Network()` and `InitialState()`. Node 0 is the outside world: jobs arrive from it and depart to it. Service nodes are numbered from 1.

```c
#include "qnet.h"

static double arrive(void)  { return Exponential(2.0); }      /* mean inter-arrival time 2.0 */
static double service(void) { return Erlang(5, 2.0 / 7.0); }

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

Add one line to `CMakeLists.txt` per variant:

```cmake
simcode_qnsim_model(mymodel     mymodel.c STEADY    FIFO)
simcode_qnsim_model(mymodel-sjf mymodel.c TRANSIENT SJF  DEFINES PLOT)
```

| Choice | Meaning |
|---|---|
| `STEADY` | `qnsim-ss.c` driver: batch means, adding batches of 1000 departures until the 95% interval is within 5% of the estimate. Reports mean jobs in the system and mean wait. |
| `TRANSIENT` | `qnsim-ts.c` driver: 10,000 replications from the initial state. Reports the mean number of external arrivals until flow in equals flow out. |
| `FIFO` / `SJF` | First-in first-out queues, or shortest job first with service times sampled on arrival |
| `DEFINES PLOT` | Transient driver prints per-replication data for plotting |
| `DEFINES JACKSON` | Model-defined switch. `example954.c` uses it to swap in exponential service. |

The built-in targets are `example954`, `example954-sjf`, `example954-jackson`, `example954-sjf-jackson`, `example954-ts`, `example954-ts-sjf`, `example954-ts-plot`, `model1` and `model2`.

The original csh wrapper still works for one-off builds: `run -h` lists its flags, and `run example954` builds and runs a model with the same C23 flags.

### Limitations

- Statistics are measured at node 0, so they are meaningful only for open networks. A closed network such as `model2.c` builds but never terminates.
- With FIFO queues, `Initial(i, j)` loads one job instead of `j`. See [Known issues](#known-issues).

## Tests

```bash
make                                   # or: ctest --preset dev
ctest --preset dev -L unit             # labels: unit, regress, sync
ctest --preset dev -R regress.ssq3     # a single case
```

- **Unit tests** (`tests/unit/`) cover the random number generator, every variate generator's mean and variance, the distribution functions against table values, the sort routine, both queue types, and the network engine.
- **Regression tests** run 52 programs and simulator variants with fixed seeds and compare their output byte-for-byte against `tests/regress/golden/`. Cases are listed in `tests/regress/cases.txt`.
- **Sync check** confirms that `qnsim/` matches the root sources.

`make golden` regenerates the expected outputs after an intended change; review the diff before committing. Builds use `-ffp-contract=off`, so results are identical at every optimization level. The golden files were produced on macOS arm64; another libm may differ in the last digits.

## What changed in 2026

**Language.** All sources compile as C23 with warnings as errors. `bool` is now the language keyword instead of a custom enum, all prototypes are explicit, helper functions are `static`, the FIFO queue macros became `static inline` functions, and C23 features such as `nullptr`, `constexpr`, `[[noreturn]]`, `[[maybe_unused]]` and `= {}` are used where they make code clearer. Macros named `INFINITY` that clashed with `<math.h>` became `NEVER`.

**Previously broken programs now build.** `MMck.c` and `monte_carlo.c` had their recorded output pasted into the source; it is now a comment, and both programs reproduce that 1997 output exactly. `sssim.c` was C++ and is now C. `mobile-server.c` is reconnected to its circular-buffer queue. `model1.c` and `model2.c` were updated to the current `Arrival`/`Service` signatures.

**Bugs fixed.** None of these changed the output of any program that ran before.
- `qnet.c`: an out-of-bounds read in `Init()`, and an infinite loop in `Initial()` with SJF queues
- `algC3.c`: both of its arrays were allocated too small
- `sssim.c`, `ex84x.c`: the wait-time array overflowed by the last few arrivals; it now grows on demand
- `simjobs.c`: the running average started from an uninitialized value
- `cdhcl.c`: the argument parser read past the end of each option string
- `circ_buf_queue.c`: `Take` could not remove from a full queue
- 1-based arrays no longer step a pointer before the start of an allocation

**Behavior changes.**
- `error()` in the simulator now writes to stderr and exits with status 1. It used stdout and status 0.
- Programs that prompt for a seed exit with a message at end of input. They used to loop forever.
- The stdin filters stop at the first unreadable value instead of using garbage. `cdhcl` rejects unknown and missing options.

## Known issues

- With FIFO queues, `Initial(i, j)` loads one job instead of `j`, because the FIFO `Insert` always adds one. The unit tests track this as a known bug.
- `mobile-server.c` has `CHECKGRAPH` defined, so it prints its routing graph and exits. Its first arrival is scheduled before the command-line arrival rate is read.
- `ssq1.c` and `sis1.c` need `.dat` input files that are not in the archive. `buffon.c` seeds from the clock, so it is not regression tested.
- `multiproc.c` is an unfinished sketch, and `simcore.c` and `triangle.c` are fragments without `main`. They are not built.

## About the files

Compiled executables from the original SunOS (SPARC) and Cygwin builds are kept on disk but not tracked by git. Files such as `qnet.c.1`, `qnsim.c.2` and `makefile~`, plus the `.tar` and `.zip` archives, are historical snapshots and are not built. The old makefiles (`makefile.mci`, `makefile.qnsim`, `makefile.bak`) and the `go` and `.run` scripts are superseded by CMake. The root `makefile` is now a wrapper around the CMake presets.

`.clang-format` describes the existing style for new code. The historical sources were deliberately not reformatted.

## Credits

- Random number, variate and statistics library (`rngs`, `rvgs`, `rvms`, `rng`) and several textbook programs by Steve Park and Dave Geyer.
- Generator from S. K. Park and K. W. Miller, "Random Number Generators: Good Ones Are Hard To Find", *Communications of the ACM*, October 1988.
- qnsim, the exercises, and the revisions noted in the source headers by Rob Jones.
