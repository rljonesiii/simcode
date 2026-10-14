# simcode -- discrete-event simulation programs (CSci 726, 1997-2002),
# modernized to C23.  See README.md.
#
#   cmake --workflow --preset dev        configure + build + test (sanitizers on)
#   cmake --workflow --preset release    optimized build + test
#   cmake --workflow --preset lint       build with clang-tidy
#
cmake_minimum_required(VERSION 3.28)

project(simcode
  VERSION 2026.10
  DESCRIPTION "Discrete-event simulation coursework and the qnsim queuing network simulator"
  LANGUAGES C)

# ---------------------------------------------------------------------------
# Language standard and project-wide options
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)          # for clang-tidy and editors
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(SIMCODE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(SIMCODE_CLANG_TIDY "Run clang-tidy while compiling" OFF)
set(SIMCODE_SANITIZERS "" CACHE STRING
    "Comma-separated sanitizers for every target, e.g. address,undefined")

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  message(FATAL_ERROR "simcode needs Clang or GCC (C23 support: Clang 18+, GCC 14+)")
endif()

add_compile_options(
  -Wall -Wextra -Wpedantic
  -Wstrict-prototypes -Wmissing-prototypes -Wshadow
  # No fused multiply-add contraction: keeps results bit-identical across
  # optimization levels, which the golden-output regression tests rely on.
  -ffp-contract=off)
if(SIMCODE_WARNINGS_AS_ERRORS)
  add_compile_options(-Werror)
endif()
if(SIMCODE_SANITIZERS)
  add_compile_options(-fsanitize=${SIMCODE_SANITIZERS} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${SIMCODE_SANITIZERS})
endif()

if(SIMCODE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy
               HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin REQUIRED)
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

find_library(MATH_LIBRARY m)

# ---------------------------------------------------------------------------
# Libraries
# ---------------------------------------------------------------------------

# Park & Geyer multi-stream library: random numbers, variates, pdf/cdf/idf,
# plus the shared Sort routine.
add_library(simlib STATIC rngs.c rvgs.c rvms.c sort.c)
target_include_directories(simlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(MATH_LIBRARY)
  target_link_libraries(simlib PUBLIC "${MATH_LIBRARY}")
endif()

# Older single-stream generator (rng.c).  It defines the same Random() as
# rngs.c, so a program links one or the other, never both.
add_library(rng_single STATIC rng.c)
target_include_directories(rng_single PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# qnsim engine, built once per queue discipline (queue.h is compile-time).
# The FIFO queue is inline in queue.h; queue.c only implements the SJF list.
set(QNET_SOURCES qnet.c qnsim.c misc.c)
add_library(qnet_fifo STATIC ${QNET_SOURCES})
target_link_libraries(qnet_fifo PUBLIC simlib)
add_library(qnet_sjf STATIC ${QNET_SOURCES} queue.c)
target_link_libraries(qnet_sjf PUBLIC simlib)
target_compile_definitions(qnet_sjf PUBLIC SJF_QUEUE)

# ---------------------------------------------------------------------------
# qnsim models
# ---------------------------------------------------------------------------
#   simcode_qnsim_model(<target> <model.c> STEADY|TRANSIENT FIFO|SJF [DEFINES ...])
function(simcode_qnsim_model target model driver discipline)
  cmake_parse_arguments(PARSE_ARGV 4 arg "" "" "DEFINES")
  if(driver STREQUAL "STEADY")
    set(driver_source "${PROJECT_SOURCE_DIR}/qnsim-ss.c")
  else()
    set(driver_source "${PROJECT_SOURCE_DIR}/qnsim-ts.c")
  endif()
  string(TOLOWER "${discipline}" discipline)
  add_executable(${target} "${driver_source}" "${model}")
  target_link_libraries(${target} PRIVATE qnet_${discipline})
  target_compile_definitions(${target} PRIVATE ${arg_DEFINES})
endfunction()

simcode_qnsim_model(example954             example954.c STEADY    FIFO)
simcode_qnsim_model(example954-sjf         example954.c STEADY    SJF)
simcode_qnsim_model(example954-jackson     example954.c STEADY    FIFO DEFINES JACKSON)
simcode_qnsim_model(example954-sjf-jackson example954.c STEADY    SJF  DEFINES JACKSON)
simcode_qnsim_model(example954-ts          example954.c TRANSIENT FIFO)
simcode_qnsim_model(example954-ts-sjf      example954.c TRANSIENT SJF)
simcode_qnsim_model(example954-ts-plot     example954.c TRANSIENT FIFO DEFINES PLOT)
simcode_qnsim_model(model1                 model1.c     STEADY    FIFO)
# model2 is a closed network; it builds, but the drivers measure flow at
# node 0 and never terminate for closed networks.
simcode_qnsim_model(model2                 model2.c     STEADY    FIFO)

# ---------------------------------------------------------------------------
# Standalone programs
# ---------------------------------------------------------------------------
#   simcode_program(<target> <sources...> [LIBS <libs...>])
function(simcode_program target)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "LIBS")
  add_executable(${target} ${arg_UNPARSED_ARGUMENTS})
  target_link_libraries(${target} PRIVATE ${arg_LIBS})
  if(MATH_LIBRARY)
    target_link_libraries(${target} PRIVATE "${MATH_LIBRARY}")
  endif()
endfunction()

# data analysis filters (read one value per line from stdin)
simcode_program(acs      acs.c)
simcode_program(bvs      bvs.c)
simcode_program(cdh      cdh.c)
simcode_program(cdhcl    cdhcl.c)
simcode_program(ddh      ddh.c)
simcode_program(uvs      uvs.c)
simcode_program(estimate estimate.c LIBS simlib)

# single- and multi-server queues, inventory systems
foreach(prog mm1 msq simjobs ssq ssq3 ssq4 sis3 sis4 sssim tsim tsimjobs)
  simcode_program(${prog} ${prog}.c LIBS simlib)
endforeach()
simcode_program(ssq1 ssq1.c)                        # needs ssq1.dat (not in archive)
simcode_program(sis1 sis1.c)                        # needs sis1.dat (not in archive)
foreach(prog ssq2 sis2 galileo buffon)
  simcode_program(${prog} ${prog}.c LIBS rng_single)
endforeach()
simcode_program(mobile-server mobile-server.c circ_buf_queue.c LIBS simlib)

# course exercises and Monte Carlo experiments
foreach(prog algC2 algC3 algC4 exC3 exC7 exF4 ex446 ex84x
             gamble monte_carlo montecarloint urn urn2)
  simcode_program(${prog} ${prog}.c LIBS simlib)
endforeach()
simcode_program(triangle-test test.c LIBS simlib)   # target name `test` is reserved
foreach(prog MMck fp gamble-eqn sieve)
  simcode_program(${prog} ${prog}.c)
endforeach()
simcode_program(sjf-queue-demo testit.c LIBS qnet_sjf)

# Not built: multiproc.c (unfinished sketch), simcore.c and triangle.c
# (code fragments without main).

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
