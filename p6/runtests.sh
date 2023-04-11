#! /bin/sh

make
rm -f output
./psort ~cs537-1/tests/P6/test-small/test-small-in.txt output 1
./rcheck ~cs537-1/tests/P6/test-small/test-small-in.txt output
rm -f output
./psort ~cs537-1/tests/P6/test-small2/test-small2-in.txt output 1
./rcheck ~cs537-1/tests/P6/test-small2/test-small2-in.txt output
rm -f output
./psort ~cs537-1/tests/P6/test-medium/test-medium-in.txt output 1
./rcheck ~cs537-1/tests/P6/test-medium/test-medium-in.txt output
rm -f output
./psort ~cs537-1/tests/P6/test-medium2/test-medium2-in.txt output 1
./rcheck ~cs537-1/tests/P6/test-medium2/test-medium2-in.txt output
rm -f output