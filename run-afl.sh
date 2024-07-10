#!/usr/bin/env bash

cd `dirname $0`

mkdir -p afl/

touch afl/tup.config
echo CONFIG_CC=afl-cc > afl/tup.config
echo CONFIG_LD=afl-cc >> afl/tup.config

cd afl

tup

mkdir -p seeds/
cp ../test/*.in ./seeds/

mkdir -p /tmp/afl-out
ln -s /tmp/afl-out afl/out # Save a bit of SSD wear...

if [ "$1" = main ] || [ "$1" = "" ]; then
  AFL_AUTORESUME=1 AFL_FINAL_SYNC=1 afl-fuzz -i seeds -o out -M main-$HOSTNAME -- ./kvds
fi

if [ "${1%-*}" = secondary ]; then
  AFL_AUTORESUME=1 AFL_FINAL_SYNC=1 afl-fuzz -i seeds -o out -S "$1" -a ascii -- ./kvds
fi
