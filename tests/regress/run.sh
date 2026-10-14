#!/bin/bash
# Golden-output regression runner.  CTest runs one case per test; the
# `golden` build target runs every case with --update.
#
#   run.sh --bin-dir DIR [--work-dir DIR] [--update] [CASE_NAME]
#
# --bin-dir   where the CMake-built executables are
# --work-dir  scratch space; each case runs in its own subdirectory
# --update    write golden files instead of comparing
# CASE_NAME   run only this case (exact name); default is every case
#
# Environment: TIMEOUT (seconds per case, default 300).
set -u

HERE=$(cd "$(dirname "$0")" && pwd)
SRC=$(cd "$HERE/../.." && pwd)
TIMEOUT=${TIMEOUT:-300}

bin=""; work=""; update=0; only=""
while [ $# -gt 0 ]; do
  case "$1" in
    --bin-dir)  bin=$2; shift 2 ;;
    --work-dir) work=$2; shift 2 ;;
    --update)   update=1; shift ;;
    -*)         echo "unknown option: $1" >&2; exit 2 ;;
    *)          only=$1; shift ;;
  esac
done
[ -n "$bin" ] || { echo "usage: run.sh --bin-dir DIR [--work-dir DIR] [--update] [CASE]" >&2; exit 2; }
bin=$(cd "$bin" && pwd)
[ -n "$work" ] || work=$(mktemp -d)
mkdir -p "$work"

resolve() {   # fixture first, then source root
  if [ -e "$HERE/fixtures/$1" ]; then echo "$HERE/fixtures/$1"; else echo "$SRC/$1"; fi
}
trim() { echo "$1" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//'; }

pass=0; fail=0; updated=0; failed_names=""; found=0
fail_case() { echo "FAIL  $1 ($2)"; fail=$((fail + 1)); failed_names="$failed_names $1"; }

while IFS='|' read -r name target input args check; do
  name=$(trim "$name")
  case "$name" in ''|'#'*) continue ;; esac
  if [ -n "$only" ] && [ "$name" != "$only" ]; then continue; fi
  found=1
  target=$(trim "$target"); input=$(trim "${input:-}")
  args=$(trim "${args:-}"); check=$(trim "${check:-}")

  exe="$bin/$target"
  [ -x "$exe" ] || { fail_case "$name" "executable $target not built"; continue; }
  dir="$work/$name"
  rm -rf "$dir"; mkdir -p "$dir"
  stdin_file=/dev/null
  [ -n "$input" ] && stdin_file=$(resolve "$input")

  # Each case runs in its own directory: some programs write side files.
  # $args is split into words on purpose.
  golden="$HERE/golden/$name.out"
  if [ "$check" = "sha256" ]; then
    golden="$HERE/golden/$name.sha256"
    # shellcheck disable=SC2086
    ( cd "$dir" && perl -e 'alarm shift; exec @ARGV' "$TIMEOUT" "$exe" $args < "$stdin_file" 2>&1 \
        | perl -MDigest::SHA -e '$d = Digest::SHA->new(256); $n = 0;
            while (<STDIN>) { $d->add($_); $n++ } printf "%s  %d lines\n", $d->hexdigest, $n' > actual.cmp
      exit "${PIPESTATUS[0]}" )
    rc=$?
  else
    # shellcheck disable=SC2086
    ( cd "$dir" && perl -e 'alarm shift; exec @ARGV' "$TIMEOUT" "$exe" $args < "$stdin_file" > actual.out 2>&1 )
    rc=$?
    cp "$dir/actual.out" "$dir/actual.cmp"
  fi

  if [ $rc -ge 128 ]; then
    if [ $rc -eq 142 ]; then fail_case "$name" "timed out after ${TIMEOUT}s"
    else fail_case "$name" "killed by signal $((rc - 128))"; fi
    continue
  fi

  if [ $update -eq 1 ]; then
    if ! cmp -s "$dir/actual.cmp" "$golden" 2>/dev/null; then
      cp "$dir/actual.cmp" "$golden"; echo "WROTE $name"; updated=$((updated + 1))
    fi
    pass=$((pass + 1))
  elif [ ! -f "$golden" ]; then
    fail_case "$name" "no golden file; build the 'golden' target"
  elif cmp -s "$golden" "$dir/actual.cmp"; then
    pass=$((pass + 1))
  else
    fail_case "$name" "output differs from golden/$(basename "$golden")"
    diff -u "$golden" "$dir/actual.cmp" | head -30 | sed 's/^/      /'
  fi
done < "$HERE/cases.txt"

if [ -n "$only" ] && [ $found -eq 0 ]; then
  echo "no case named '$only' in cases.txt"; exit 2
fi
if [ $update -eq 1 ]; then
  echo "regression: $pass cases run, $updated golden files written, $fail failed"
else
  echo "regression: $pass passed, $fail failed"
fi
[ $fail -eq 0 ] || { echo "failed:$failed_names"; exit 1; }
