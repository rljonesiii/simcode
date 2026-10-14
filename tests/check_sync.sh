#!/bin/bash
# Verify that the distributable copy in qnsim/ matches the root sources.
# The root `copy` script is what keeps them in sync. Whitespace-only
# differences are ignored (qnsim/run has trailing spaces on one help line).
ROOT=${1:-$(cd "$(dirname "$0")/.." && pwd)}
status=0; n=0
for f in "$ROOT"/qnsim/*.c "$ROOT"/qnsim/*.h "$ROOT"/qnsim/run; do
  base=$(basename "$f"); n=$((n + 1))
  if [ ! -f "$ROOT/$base" ]; then
    echo "SYNC  qnsim/$base has no counterpart in the root"; status=1
  elif ! diff -bq "$f" "$ROOT/$base" > /dev/null; then   # ignore whitespace-only drift
    echo "SYNC  qnsim/$base differs from $base (run ./copy from the root)"; status=1
  fi
done
[ $status -eq 0 ] && echo "sync: $n qnsim/ files match the root"
exit $status
