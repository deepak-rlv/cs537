#! /bin/sh

make
echo "Small"
./psort tests/test-small/test-small-in.txt output $1
./tests/rcheck tests/test-small/test-small-in.txt output
diff -sq output tests/test-small/test-small-out.txt

echo "Small 2"
./psort tests/test-small2/test-small2-in.txt output $1
./tests/rcheck tests/test-small2/test-small2-in.txt output
diff -sq output tests/test-small2/test-small2-out.txt

echo "Medium"
./psort tests/test-medium/test-medium-in.txt output $1
./tests/rcheck tests/test-medium/test-medium-in.txt output
diff -sq output tests/test-medium/test-medium-out.txt

echo "Medium 2"
./psort tests/test-medium2/test-medium2-in.txt output $1
./tests/rcheck tests/test-medium2/test-medium2-in.txt output
diff -sq output tests/test-medium2/test-medium2-out.txt

echo "Large"
./psort tests/test-large/test-large.in output $1
./tests/rcheck tests/test-large/test-large.in output
diff -sq output tests/test-large/test-large.out

echo "Large 2"
./psort tests/test-large2/test-large2.in output $1
./tests/rcheck tests/test-large2/test-large2.in output
diff -sq output tests/test-large2/test-large2.out
