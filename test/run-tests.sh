#!/usr/bin/env bash

set -e
set -o pipefail

cd "`dirname $0`"

ALGO="${1:-default}"

KVDS=../bin/kvds

echo "ALGO: $ALGO"

for f in ./*.in; do
  o=${f%.in}.out
  echo "TEST: $f"
  git diff --no-index $o <(cat $f | $KVDS $ALGO)
done

echo "DONE: all tests passed!"
