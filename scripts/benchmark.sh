#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_PATH="${1:-$ROOT_DIR/person-info-md5-hash.c}"
RUNS="${2:-1}"
BIN_PATH="$ROOT_DIR/.bench-bin"

if [[ ! -f "$SRC_PATH" ]]; then
  echo "source file not found: $SRC_PATH" >&2
  exit 1
fi

cc -O2 -pthread "$SRC_PATH" -o "$BIN_PATH"

sum=0
for ((i = 1; i <= RUNS; i++)); do
  time_output="$(/usr/bin/time -p "$BIN_PATH" 2>&1 >/dev/null)"
  real_seconds="$(printf '%s\n' "$time_output" | awk '/^real / { print $2 }')"
  if [[ -z "$real_seconds" ]]; then
    echo "failed to read runtime for run $i" >&2
    exit 1
  fi

  printf 'run %d: %ss\n' "$i" "$real_seconds"
  sum="$(awk -v a="$sum" -v b="$real_seconds" 'BEGIN { printf "%.6f", a + b }')"
done

avg="$(awk -v total="$sum" -v n="$RUNS" 'BEGIN { printf "%.6f", total / n }')"
printf 'avg: %ss\n' "$avg"

rm -f "$BIN_PATH"
