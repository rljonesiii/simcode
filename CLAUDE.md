# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Discrete-event simulation coursework (CSci 726, 1997, revised 2001–2002) by Rob Jones, built on the Park & Geyer simulation library and modernized to C23 in 2026. The root is a flat directory: current sources sit next to historical snapshots (`qnet.c.1`, `qnsim.c.2`, `makefile~`), tarballs, data files and old binaries that `.gitignore` excludes. Only files listed in `CMakeLists.txt` are built.

`qnsim/` is a distributable copy of the simulator and must stay identical to the root files of the same name. After editing any of them, run `sh ./copy` from the root; the `sync.qnsim` test fails otherwise.

## Build and test

CMake with presets; `make` at the root is a thin wrapper.

```bash
make                     # = cmake --workflow --preset dev (debug + ASan/UBSan + all tests)
make release             # optimized build + tests
make lint                # build through clang-tidy (.clang-tidy, WarningsAsErrors)
make golden              # regenerate tests/regress/golden/ (review the diff)
ctest --preset dev -L unit                 # labels: unit, regress, sync
ctest --preset dev -R regress.qnsim_ss_sjf # one regression case
build/dev/bin/test_rvms student            # one unit binary, tests whose name contains "student"
```

Build trees are `build/<preset>/`, executables in `build/<preset>/bin/`. clang-tidy and current clang-format come from Homebrew LLVM (`/opt/homebrew/opt/llvm/bin`); the `clang-format` on PATH is version 10 and too old for `.clang-format`.

Compiler flags for every target: `-std=c23 -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -Wshadow -ffp-contract=off -Werror`. Consequences:
- Every non-`main` function in a program must be `static` or have a prototype in a header.
- `-ffp-contract=off` is what makes outputs bit-identical across Debug and Release. Do not remove it, or the golden files stop matching at higher optimization.

## Rules for changing code

- **Output is the contract.** Any change to a program must leave its regression output byte-identical unless the change is meant to alter results. When a fix legitimately changes output, regenerate with `make golden` and say so.
- `bool`/`true`/`false` are C23 keywords. Do not reintroduce `TRUE`/`FALSE` or a `bool` typedef.
- `INFINITY` from `<math.h>` is not redefined anywhere. Programs use `NEVER` for "no event scheduled".
- 1-based arrays are allocated with one extra element (`calloc(n + 1, ...)`). Never use the old `p = &p[-1]` idiom.
- Do not mass-reformat historical sources. `.clang-format` is for new code and substantial rewrites.

## Tests

- `tests/unit/minitest.h` is a self-contained harness. `CHECK_XFAIL` marks a known bug: it passes while the bug exists and fails with "XPASS" once fixed, after which it must become `CHECK`.
- `test_qnet.c` runs each test in a forked child (`mt_isolate`) because qnet.c keeps global state, reports user errors with `error()` (stderr, exit 1), and could hang. `test_queue.c` and `test_qnet.c` are each built twice, linked against `qnet_fifo` and `qnet_sjf`.
- `tests/regress/cases.txt` maps a case name to a CMake target, a stdin file, arguments, and a check mode. CTest creates one `regress.<name>` test per line. Stdin files resolve in `tests/regress/fixtures/` first, then the root. `fixtures/seed.txt` answers the seed prompt of programs that call `PlantSeeds(0)`. Large outputs use the `sha256` check mode, which stores a digest and line count.
- To add a program to regression: add a target in `CMakeLists.txt`, add a line to `cases.txt`, reconfigure, then `make golden`.
- Reference points for correctness: `algC2`'s output matches the 2002 `output` file; `MMck` and `monte_carlo` match the 1997 output in the comment at the end of their sources.

## Architecture

### Libraries (CMakeLists.txt)
- `simlib`: `rngs.c` (256-stream Lehmer generator: `PlantSeeds`, `SelectStream`, `Random`), `rvgs.c` (variates), `rvms.c` (pdf/cdf/idf, e.g. `idfStudent` for intervals) and `sort.c`.
- `rng_single`: `rng.c`, the older single-stream generator. It defines the same `Random()` as `rngs.c`, so a program links one or the other.
- `qnet_fifo` / `qnet_sjf`: the qnsim engine. The queue discipline is compile-time (`SJF_QUEUE`), so the engine is built twice; `qnet_sjf` exports the define.

### qnsim
- `qnet.h` declares the shared network state (`type`, `num_nodes`, `queue`, `status`, `random_time`) and the model API: `Nodes`, `Connect`, `Arrival`, `Service`, `Initial`. Node 0 is the external source/sink, so `Nodes(n)` allocates `n + 1` slots and `Init()` turns each routing row of `P` into cumulative probabilities.
- `queue.h`: FIFO is a job counter implemented as `static inline` functions. `SJF_QUEUE` switches to the sorted list in `queue.c`, which stores service times sampled at arrival.
- `qnsim.c`: `ProcessArrival` / `ServiceJob`. Node `i` uses random stream `i`.
- `qnsim-ss.c` (batch means) and `qnsim-ts.c` (transient replications) are alternative `main()` drivers. Statistics are taken at node 0, so only open networks terminate.
- A model file defines `Network()`, `InitialState()` and `static` time functions. `simcode_qnsim_model()` in `CMakeLists.txt` builds a model with a chosen driver and queue discipline.

### Other programs
Each standalone program is one `.c` file registered with `simcode_program()`. `mobile-server.c` also links `circ_buf_queue.c`. `test.c` builds as `triangle-test` because CMake reserves the target name `test`. Not built: `multiproc.c` (unfinished), `simcore.c` and `triangle.c` (fragments).

## Known open issues
- FIFO `Initial(i, j)` loads one job, not `j`: the FIFO `Insert` adds one regardless of the value. Tracked by `CHECK_XFAIL` in `test_qnet.c`; the `qnsim_ss_fifo_initial` golden reflects it, so a fix must update both.
- `model2.c` is a closed network and never terminates with either driver.
- `mobile-server.c` has `CHECKGRAPH` defined (prints the graph and exits), and schedules its first arrival before reading the arrival rate from the command line.
- `ssq1.c` and `sis1.c` need missing `.dat` files; `buffon.c` is clock-seeded.

## Notes
`.notes` holds recorded results for Exercise 9.3.13 (gambler's ruin, simulation vs. a Markov chain solver). `view.gps`/`view.gpc` are gnuplot scripts for program output.
