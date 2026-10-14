#!/bin/bash
# Golden-output regression runner for simcode.  Usually invoked via
# `make regress` / `make golden` from tests/.
#
#   run.sh [--update] [NAME_SUBSTRING]
#
# Environment: SRC (source root), BUILD (scratch dir), CC, TIMEOUT (seconds).
set -u

HERE=$(cd "$(dirname "$0")" && pwd)
SRC=$(cd "${SRC:-$HERE/../..}" && pwd)
BUILD=${BUILD:-$HERE/../build/regress}
CC=${CC:-cc}
TIMEOUT=${TIMEOUT:-60}
CSTD="-std=gnu89 -w"

update=0
filter=""
for arg in "$@"; do
  case "$arg" in
    --update) update=1 ;;
    *)        filter="$arg" ;;
  esac
done

resolve() {   # fixture first, then source root
  if [ -e "$HERE/fixtures/$1" ]; then echo "$HERE/fixtures/$1"; else echo "$SRC/$1"; fi
}
trim() { echo "$1" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//'; }

mkdir -p "$BUILD"
pass=0; fail=0; updated=0; failed_names=""

while IFS='|' read -r name sources cflags input check; do
  name=$(trim "$name")
  case "$name" in ''|'#'*) continue ;; esac
  if [ -n "$filter" ] && [[ "$name" != *"$filter"* ]]; then continue; fi
  sources=$(trim "$sources"); cflags=$(trim "$cflags"); input=$(trim "${input:-}"); check=$(trim "${check:-}")

  dir="$BUILD/$name"
  rm -rf "$dir"; mkdir -p "$dir"
  paths=""
  for s in $sources; do paths="$paths $(resolve "$s")"; done

  # shellcheck disable=SC2086
  if ! "$CC" $CSTD $cflags -I"$SRC" $paths -lm -o "$dir/prog" > "$dir/build.log" 2>&1; then
    echo "FAIL  $name (build failed)"; sed 's/^/      /' "$dir/build.log" | head -20
    fail=$((fail + 1)); failed_names="$failed_names $name"; continue
  fi

  stdin_file=/dev/null
  [ -n "$input" ] && stdin_file=$(resolve "$input")

  # run inside its own dir: some programs write side files (e.g. ddh plot data)
  golden="$HERE/golden/$name.out"
  if [ "$check" = "sha256" ]; then
    # stream the (huge) output into "<sha256>  <N> lines" without storing it
    golden="$HERE/golden/$name.sha256"
    ( cd "$dir" && perl -e 'alarm shift; exec @ARGV' "$TIMEOUT" ./prog < "$stdin_file" 2>&1 \
        | perl -MDigest::SHA -e '$d = Digest::SHA->new(256); $n = 0;
            while (<STDIN>) { $d->add($_); $n++ } printf "%s  %d lines\n", $d->hexdigest, $n' > actual.cmp
      exit "${PIPESTATUS[0]}" )
    rc=$?
  else
    ( cd "$dir" && perl -e 'alarm shift; exec @ARGV' "$TIMEOUT" ./prog < "$stdin_file" > actual.out 2>&1 )
    rc=$?
    cp "$dir/actual.out" "$dir/actual.cmp"
  fi

  if [ $rc -ge 128 ]; then
    if [ $rc -eq 142 ]; then why="timed out after ${TIMEOUT}s"; else why="killed by signal $((rc - 128))"; fi
    echo "FAIL  $name ($why)"
    fail=$((fail + 1)); failed_names="$failed_names $name"; continue
  fi

  if [ $update -eq 1 ]; then
    if ! cmp -s "$dir/actual.cmp" "$golden" 2>/dev/null; then
      cp "$dir/actual.cmp" "$golden"; echo "WROTE $name"; updated=$((updated + 1))
    fi
    pass=$((pass + 1)); continue
  fi

  if [ ! -f "$golden" ]; then
    echo "FAIL  $name (no golden file; run 'make golden CASE=$name')"
    fail=$((fail + 1)); failed_names="$failed_names $name"
  elif cmp -s "$golden" "$dir/actual.cmp"; then
    pass=$((pass + 1))
  else
    echo "FAIL  $name (output differs from golden/$(basename "$golden"))"
    diff -u "$golden" "$dir/actual.cmp" | head -30 | sed 's/^/      /'
    fail=$((fail + 1)); failed_names="$failed_names $name"
  fi
done < "$HERE/cases.txt"

if [ $update -eq 1 ]; then
  echo "regression: $pass cases run, $updated golden files written, $fail failed"
else
  echo "regression: $pass passed, $fail failed"
fi
[ $fail -eq 0 ] || { echo "failed:$failed_names"; exit 1; }
