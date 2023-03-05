# **Project 4**

## **XV6 Scheduler**

Submitted by Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
<br> <br>
**CS Login:** ```logavaseekaran```
<br>
**Wisc ID:** 908 495 9841
<br>
**Status:** All 17 tests passing
<br> <br>

## **Implementation**

A stride scheduler as per the algorithm mentioned in [this paper](https://web.eecs.umich.edu/~mosharaf/Readings/Stride.pdf) is implemented in the XV6 kernel

<br>

Two functions ```int settickets(int number)``` and ```int getpinfo(struct pstat *)``` are implemented for setting the tickets of a process and getting information about all processes respectively.