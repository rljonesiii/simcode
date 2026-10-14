# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An archive of ANSI C discrete-event simulation coursework (CSci 726, 1997, with some 2001–2002 revisions) by Rob Jones, built on the Park & Geyer "Discrete-Event Simulation" library. It is not a git repository. The root directory is a flat working area mixing sources, stale binaries (`*.exe`, `*.o`, `a.out`, Cygwin `*.stackdump`), output files, numbered backups (`qnsim.c.1`, `qnet.h.2`, `makefile~`) and tarballs. Treat binaries, `.o` files, `*.out`, and the numbered/`~` backups as artifacts, not sources.

`qnsim/` is the clean, distributable copy of the queuing network simulator. Its files are identical to the same-named files in the root; the root `copy` script is what syncs them. If you change a qnsim source in one place, keep the other in sync.

## Building on modern macOS / clang

The code is pre-C99 (implicit declarations, a custom `bool` enum in `misc.h`). Default clang rejects it, so always compile with:

```
-std=gnu89 -w
```

### Queuing network simulator (qnsim)

The csh driver `run` (also in `qnsim/`) compiles and runs a model. `run -h` prints usage. It calls plain `gcc` without `-std`, so on current clang build directly instead:

```bash
gcc -std=gnu89 -w rngs.c rvgs.c rvms.c queue.c misc.c qnet.c qnsim.c qnsim-ss.c example954.c -lm -o example954 && ./example954
```

`run` flag → compile-time define / file swap:
- `-t` → use `qnsim-ts.c` (transient) instead of `qnsim-ss.c` (steady-state, batch means)
- `-sjf` → `-DSJF_QUEUE` (shortest-job-first queue instead of FIFO)
- `-x` → `-DJACKSON` (model-defined alternative, e.g. exponential service in `example954.c`)
- `-p` → `-DPLOT` (plot-friendly output in the transient driver)
- `-d` → `-DDEBUG -g`

### Other programs

Each standalone program is a single `.c` file linked against the library modules it includes, e.g.:

```bash
gcc -std=gnu89 -w ssq2.c rngs.c -lm -o ssq2
gcc -std=gnu89 -w sssim.c rngs.c rvgs.c rvms.c sort.c -lm -o sssim     # what `makefile` builds
gcc -std=gnu89 -w montecarloint.c rngs.c rvgs.c rvms.c -lm -o mci      # what `makefile.mci` builds
gcc -std=gnu89 -w mobile-server.c rngs.c rvgs.c rvms.c queue.c -lm -o mssim   # what `go` does
```

`makefile` and `makefile.mci` use `g++` and link `-lfl` (flex), which is not needed and usually absent; drop it. `makefile.qnsim`/`makefile.bak` use a broken `$1` target and are obsolete. Data-analysis tools (`uvs`, `bvs`, `acs`, `cdh`, `ddh`) read one value per line from stdin, e.g. `./ddh < ddh.dat`.

## Tests

The suite lives in `tests/` and is driven by `tests/Makefile`. Run it from that directory:

```bash
make test
```

- `make unit` builds and runs the unit tests with AddressSanitizer and UndefinedBehaviorSanitizer (`SANITIZE=` disables them). One binary, optionally filtered to tests whose name contains a word: `make build/unit/test_rvms && build/unit/test_rvms student`.
- `make regress` builds 37 programs with fixed seeds and compares their output byte-for-byte with `tests/regress/golden/`. `make regress CASE=qnsim_ss` runs a subset.
- `make sync` checks that `qnsim/` matches the root sources, ignoring whitespace.
- `make golden` regenerates golden files. Only do this when an output change is intended, and review the diff. Goldens were made on macOS arm64 with Apple clang, so another platform may need regenerating.
- `make unit regress SRC=/some/copy BUILD=/tmp/x` runs the suite against a different source tree, which is how fault-injection checks were done.

How it is put together:
- `tests/unit/minitest.h` is a self-contained harness. `CHECK_XFAIL` marks a known bug: it passes while the bug exists and fails with "XPASS" once fixed, so the marker must then become `CHECK`.
- `test_qnet.c` runs every test in a forked child (`mt_isolate`) because qnet.c uses globals, reports user errors by calling `exit(0)`, and could hang. `test_queue.c` and `test_qnet.c` are each built twice, as FIFO and with `-DSJF_QUEUE`.
- `tests/regress/cases.txt` lists each case as name, sources, extra flags, stdin file, and check mode. Sources resolve in `tests/regress/fixtures/` first, then the root. `fixtures/seed.txt` answers the seed prompt of programs that call `PlantSeeds(0)`. The `-DPLOT` case is 14.8M lines, so it is checked by SHA-256 digest (`check` column `sha256`).
- The `algC2` golden matches the 2002 `output` file in the root exactly.

Programs not covered because they do not build or run unattended: `MMck.c`, `monte_carlo.c` (stray text in source), `multiproc.c` (undeclared identifiers), `mobile-server.c` (its `bool` enum clashes with `misc.h`), `sssim.c` (C++, uses `new`), `model1.c`/`model2.c` (older `Arrival`/`Service` signatures without the rate argument), `ssq1.c`/`sis1.c` (their `.dat` input files are missing), `algC3.c` (segfaults at exit), `buffon.c` (clock-seeded), `gamble.c` (about 9 s), and `exF4`/`urn`/`urn2`/`test` (very large output).

## Architecture

### Library layer (Park & Geyer, do not modify casually)
- `rngs.c/h`: multi-stream Lehmer RNG (`PlantSeeds`, `SelectStream`, `Random`). `rng.c/h` is the older single-stream version used by some early programs; don't link both.
- `rvgs.c/h`: random variate generators (`Exponential`, `Erlang`, `Normal`, ...).
- `rvms.c/h`: pdf/cdf/idf functions, used for confidence intervals (`idfStudent`).

### Queuing network simulator
Split into a reusable engine and a user-supplied model file:
- `qnet.c/h`: global network state (`num_nodes`, routing matrix `P`, per-node `random_time` function pointers, `status`, `queue`) and the model-building API: `Nodes`, `Connect`, `Arrival`, `Service`, `Initial`. Node 0 is the external source/sink, so `Nodes(n)` allocates `n + 1` slots and `Init()` converts `P` rows to cumulative probabilities.
- `queue.c/h`: compile-time-selected queue. FIFO (default) is just a job counter implemented as macros. `SJF_QUEUE` is a sorted linked list storing pre-sampled service times.
- `qnsim.c/h`: shared event helpers `ProcessArrival` / `ServiceJob`. RNG stream `i` is used for node `i`.
- `qnsim-ss.c` / `qnsim-ts.c`: alternative `main()` drivers (steady-state batch means vs. transient replications). Exactly one is linked. Statistics are referenced to node 0, so they only make sense for open networks.
- Model file (e.g. `example954.c`): must define `Network()` and `InitialState()`, and the service/arrival functions passed to `Service`/`Arrival`.

### Fixed bugs in qnet.c (2026-10-14, both copies)
- `Init()` checked `random_time[i]` with `i <= num_nodes`, reading one past the array. The FIFO build aborted with "Must specify a service-time function for every service node." Now `<`.
- `Initial()` under `SJF_QUEUE` incremented `j` instead of `k`, looping forever when a node started non-empty.

The backup files `qnet.c.1`, `qnet.c.2` and the tarballs still contain the old code.

### Open bugs
- **FIFO `Initial(i, j)` loads one job, not `j`.** The FIFO `Insert` macro in `queue.h` always increments by one. The unit tests mark this with `CHECK_XFAIL`, and the `qnsim_ss_fifo_initial` golden reflects the buggy behavior, so fixing it means updating both.
- `error(MEM_ERR)` passes the integer 0 where `error()` expects a string.
- `circ_buf_queue.c` includes `queue.h` instead of `circ_buf_queue.h`. Nothing uses it.

### Results / notes
`.notes` holds recorded results for Exercise 9.3.13 (gambler's-ruin simulation vs. a Markov chain solver), useful as reference values. `view.gps`/`view.gpc` and `cdh.gpd` are plotting scripts for program output.
