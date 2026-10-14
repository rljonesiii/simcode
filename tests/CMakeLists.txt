# Test suite: unit tests, golden-output regression tests, qnsim/ sync check.
#
#   ctest --preset dev                   everything
#   ctest --preset dev -L unit           unit tests only (labels: unit, regress, sync)
#   ctest --preset dev -R regress.ssq3   one regression case
#   cmake --build --preset dev --target golden   regenerate golden outputs

find_program(BASH_EXE bash REQUIRED)
find_program(PERL_EXE perl REQUIRED)          # used by regress/run.sh

# ---- unit tests -----------------------------------------------------------
function(simcode_unit_test target source)
  cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "LIBS")
  add_executable(${target} "${source}")
  target_include_directories(${target} PRIVATE unit)
  target_link_libraries(${target} PRIVATE ${arg_LIBS})
  add_test(NAME unit.${target} COMMAND ${target})
  set_tests_properties(unit.${target} PROPERTIES
    LABELS unit
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")   # 1997 code never frees
endfunction()

simcode_unit_test(test_rngs       unit/test_rngs.c  LIBS simlib)
simcode_unit_test(test_rvgs       unit/test_rvgs.c  LIBS simlib)
simcode_unit_test(test_rvms       unit/test_rvms.c  LIBS simlib)
simcode_unit_test(test_sort       unit/test_sort.c  LIBS simlib)
simcode_unit_test(test_queue_fifo unit/test_queue.c LIBS qnet_fifo)
simcode_unit_test(test_queue_sjf  unit/test_queue.c LIBS qnet_sjf)
simcode_unit_test(test_qnet_fifo  unit/test_qnet.c  LIBS qnet_fifo)
simcode_unit_test(test_qnet_sjf   unit/test_qnet.c  LIBS qnet_sjf)

# ---- regression fixtures --------------------------------------------------
set(fixture "${CMAKE_CURRENT_SOURCE_DIR}/regress/fixtures/initial_model.c")
simcode_qnsim_model(initial-model        "${fixture}" STEADY    FIFO)
simcode_qnsim_model(initial-model-sjf    "${fixture}" STEADY    SJF)
simcode_qnsim_model(initial-model-ts-sjf "${fixture}" TRANSIENT SJF)

# ---- regression tests: one CTest test per line of regress/cases.txt -------
set(runner "${CMAKE_CURRENT_SOURCE_DIR}/regress/run.sh")
set(bin_dir "$<TARGET_FILE_DIR:example954>")
set(work_dir "${CMAKE_CURRENT_BINARY_DIR}/regress")

file(STRINGS regress/cases.txt case_lines REGEX "^[A-Za-z]")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS regress/cases.txt)
set(regress_targets "")
foreach(line IN LISTS case_lines)
  string(REGEX MATCH "^([^ |]+) *\\| *([^ |]+)" _ "${line}")
  set(case_name "${CMAKE_MATCH_1}")
  list(APPEND regress_targets "${CMAKE_MATCH_2}")
  add_test(NAME regress.${case_name}
           COMMAND "${BASH_EXE}" "${runner}" --bin-dir "${bin_dir}"
                   --work-dir "${work_dir}" "${case_name}")
  set_tests_properties(regress.${case_name} PROPERTIES
    LABELS regress
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
endforeach()
list(REMOVE_DUPLICATES regress_targets)

add_custom_target(golden
  COMMAND "${BASH_EXE}" "${runner}" --update --bin-dir "${bin_dir}" --work-dir "${work_dir}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Regenerating golden outputs (review the git diff before committing)"
  VERBATIM)
add_dependencies(golden ${regress_targets})

# ---- the distributable qnsim/ copy must match the root sources ------------
add_test(NAME sync.qnsim COMMAND "${BASH_EXE}" "${CMAKE_CURRENT_SOURCE_DIR}/check_sync.sh"
                                 "${PROJECT_SOURCE_DIR}")
set_tests_properties(sync.qnsim PROPERTIES LABELS sync)
