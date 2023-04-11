#! /bin/sh

make
./psort tests/test-small/test-small-in.txt output $1
./tests/rcheck tests/test-small/test-small-in.txt output

./psort tests/test-small2/test-small2-in.txt output $1
./tests/rcheck tests/test-small2/test-small2-in.txt output

./psort tests/test-medium/test-medium-in.txt output $1
./tests/rcheck tests/test-medium/test-medium-in.txt output

./psort tests/test-medium2/test-medium2-in.txt output $1
./tests/rcheck tests/test-medium2/test-medium2-in.txt output

