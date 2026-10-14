# simcode test suite.  Run from this directory:
#
#   make test                    unit + regression + qnsim/ sync check
#   make unit                    unit tests only (built with ASan/UBSan)
#   make regress                 golden-output regression tests only
#   make regress CASE=qnsim_ss   only regression cases whose name contains CASE
#   make golden                  regenerate golden outputs (review the diff!)
#   make sync                    check qnsim/ copies match the root sources
#   make clean
#
# One unit test binary, optionally one test inside it:
#   make build/unit/test_rvms && build/unit/test_rvms student
#
# SANITIZE= (empty) builds the unit tests without sanitizers.

SRC      ?= ..
BUILD    ?= build
CSTD      = -std=gnu89 -w
SANITIZE ?= -fsanitize=address,undefined -fno-omit-frame-pointer
UCFLAGS   = $(CSTD) -g $(SANITIZE) -I$(SRC) -Iunit
U         = $(BUILD)/unit
HDR       = unit/minitest.h

QNET_SRC = $(SRC)/qnet.c $(SRC)/qnsim.c $(SRC)/queue.c $(SRC)/misc.c \
           $(SRC)/rngs.c $(SRC)/rvgs.c $(SRC)/rvms.c

UNIT_BINS = $(U)/test_rngs $(U)/test_rvgs $(U)/test_rvms $(U)/test_sort \
            $(U)/test_queue_fifo $(U)/test_queue_sjf \
            $(U)/test_qnet_fifo $(U)/test_qnet_sjf

# ASan's leak checker is noisy for these 1997 programs (they never free).
export ASAN_OPTIONS = detect_leaks=0

.PHONY: test unit regress golden sync clean

test: unit regress sync

unit: $(UNIT_BINS)
	@status=0; for t in $(UNIT_BINS); do $$t || status=1; done; exit $$status

$(U):
	mkdir -p $@

$(U)/test_rngs: unit/test_rngs.c $(SRC)/rngs.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_rngs.c $(SRC)/rngs.c -lm -o $@

$(U)/test_rvgs: unit/test_rvgs.c $(SRC)/rvgs.c $(SRC)/rngs.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_rvgs.c $(SRC)/rvgs.c $(SRC)/rngs.c -lm -o $@

$(U)/test_rvms: unit/test_rvms.c $(SRC)/rvms.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_rvms.c $(SRC)/rvms.c -lm -o $@

$(U)/test_sort: unit/test_sort.c $(SRC)/sort.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_sort.c $(SRC)/sort.c -lm -o $@

$(U)/test_queue_fifo: unit/test_queue.c $(SRC)/queue.c $(SRC)/queue.h $(SRC)/misc.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_queue.c $(SRC)/queue.c $(SRC)/misc.c -lm -o $@

$(U)/test_queue_sjf: unit/test_queue.c $(SRC)/queue.c $(SRC)/queue.h $(SRC)/misc.c $(HDR) | $(U)
	$(CC) $(UCFLAGS) -DSJF_QUEUE unit/test_queue.c $(SRC)/queue.c $(SRC)/misc.c -lm -o $@

$(U)/test_qnet_fifo: unit/test_qnet.c $(QNET_SRC) $(SRC)/qnet.h $(SRC)/queue.h $(HDR) | $(U)
	$(CC) $(UCFLAGS) unit/test_qnet.c $(QNET_SRC) -lm -o $@

$(U)/test_qnet_sjf: unit/test_qnet.c $(QNET_SRC) $(SRC)/qnet.h $(SRC)/queue.h $(HDR) | $(U)
	$(CC) $(UCFLAGS) -DSJF_QUEUE unit/test_qnet.c $(QNET_SRC) -lm -o $@

regress:
	@SRC="$(abspath $(SRC))" BUILD="$(abspath $(BUILD))/regress" CC="$(CC)" \
	  ./regress/run.sh $(CASE)

golden:
	@SRC="$(abspath $(SRC))" BUILD="$(abspath $(BUILD))/regress" CC="$(CC)" \
	  ./regress/run.sh --update $(CASE)

sync:
	@./check_sync.sh "$(abspath $(SRC))"

clean:
	rm -rf $(BUILD)
