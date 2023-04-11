#! /bin/sh

make
rm -f output
./psort tests/test-small/test-small-in.txt output 1
./rcheck tests/test-small/test-small-in.txt output
rm -f output
./psort tests/test-small2/test-small2-in.txt output 1
./rcheck tests/test-small2/test-small2-in.txt output
rm -f output
./psort tests/test-medium/test-medium-in.txt output 1
./rcheck tests/test-medium/test-medium-in.txt output
rm -f output
./psort tests/test-medium2/test-medium2-in.txt output 1
./rcheck tests/test-medium2/test-medium2-in.txt output
rm -f output
