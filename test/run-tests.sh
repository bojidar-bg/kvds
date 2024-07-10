#!/usr/bin/env bash

set -e
set -o pipefail

cd "`dirname $0`"

KVDS=../bin/kvds

for f in ./*.in; do
  o=${f%.in}.out
  echo "TEST: $f"
  git diff --no-index $o <(cat $f | $KVDS)
done
