#! /bin/env python

import toolspath
from testing import Xv6Build, Xv6Test

###
# Tests for Project 5: Copy-on-Write Fork
# Part 1: Tests for getNumFreePages() in single process: init, increase, decrease
# Part 2: Tests for copy-on-write fork (using freepages): 
#       [1 fork-ro, 1 fork write before fork, 1 fork-rw in parent/child, multiple forks with reads/writes in parent/child]
# Part 3: Tests for copy-on-write fork (by checking actual copying using changes variables in address space):
#      [eg: change a variable in parent/child and make sure it doesn't change in other process]
# Part 4: Tests for reference counting
###


class test1(Xv6Test):
   name = "test_1"
   description = "Test to make sure that the number of free pages is greater than 0"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test2(Xv6Test):
   name = "test_2"
   description = "Test that sbrk() reduces the number of free pages"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test3(Xv6Test):
   name = "test_3"
   description = "Test that pages are copied on write and that the parent values are not affected by the child"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test4(Xv6Test):
   name = "test_4"
   description = "Test that all pages are not copied when process forks - 1"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test5(Xv6Test):
   name = "test_5"
   description = "Test that all pages are not copied when process forks - 2"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test6(Xv6Test):
   name = "test_6"
   description = "Test that pages copied are equal to the pages written to"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test7(Xv6Test):
   name = "test_7"
   description = "Test that multiple copies are created when pages shared by more processes are modified"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test8(Xv6Test):
   name = "test_8"
   description = "Test that pages copied are equal to the pages written to - Modify non-contiguous pages"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test9(Xv6Test):
   name = "test_9"
   description = "Test reference counting for CoW"
   tester = "ctests/" + name + ".c"
   point_value = 1

class test10(Xv6Test):
   name = "test_10"
   description = "Test reference counting for CoW- new allocations from child don't affect parent"
   tester = "ctests/" + name + ".c"
   point_value = 1

import toolspath
from testing.runtests import main
main(Xv6Build, all_tests=[test1, test2, test3, test4, test5, test6, test7, test8, test9, test10])
