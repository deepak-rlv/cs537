# **Project 2**

## **xv6 Intro**

Submitted by Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
<br> <br>
**CS Login:** ```logavaseekaran```
<br>
**Wisc ID:** 908 495 9841
<br>
**Status:** All 8 tests passing
<br> <br>

## **Implementation**

Added 2 new system calls to xv6, <br>
``` int getnextpid() ``` to get the next available process ID (PID), ``` nextpid ``` from ``` proc.c ``` <br>

``` int getprocstate(int pid, char* state, int n) ``` to find the state of the first process with the given ``` pid ```

<br>

A small program to test the implementation is also included in the userspace, named ``` p2test.c ```

<br>

To run the xv6 OS on emulator, ``` make qemu-nox ```. Once booted, run ```p2test``` to execute the test program which prints the process ID of the previous process ( fetched using ``` int getnextpid() ``` ) and the running state.