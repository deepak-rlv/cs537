#!/bin/sh

make
./runscan test_disk_images/test_0/image-02 output
echo "*********TEST 0 RESULTS*********"
python3 rcheck.py output test_disk_images/test_0/output
make
./runscan test_disk_images/test_1/01.img output
echo "*********TEST 1 RESULTS*********"
python3 rcheck.py output test_disk_images/test_1/output
make
./runscan test_disk_images/test_2/02.img output
echo "*********TEST 2 RESULTS*********"
python3 rcheck.py output test_disk_images/test_2/output
make
./runscan test_disk_images/test_3/03.img output
echo "*********TEST 3 RESULTS*********"
python3 rcheck.py output test_disk_images/test_3/output
make
./runscan test_disk_images/test_4/04.img output
echo "*********TEST 4 RESULTS*********"
python3 rcheck.py output test_disk_images/test_4/output
make
./runscan test_disk_images/test_5/05.img output
echo "*********TEST 5 RESULTS*********"
python3 rcheck.py output test_disk_images/test_5/output
make
./runscan test_disk_images/test_6/06.img output
echo "*********TEST 6 RESULTS*********"
python3 rcheck.py output test_disk_images/test_6/output
make
./runscan test_disk_images/test_7/07.img output
echo "*********TEST 7 RESULTS*********"
python3 rcheck.py output test_disk_images/test_7/output