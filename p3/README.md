# **Project 3**

## **Unix Shell**

Submitted by Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
<br> <br>
**CS Login:** ```logavaseekaran```
<br>
**Wisc ID:** 908 495 9841
<br>
**Status:** All 13 tests passing
<br> <br>

## **Implementation**
A simple shell implementation with ```exit```, ```cd```, ```pwd``` and ```loop``` as in-built commands, with support for multiple commands ```;``` redirection ```>``` and pipes ```|```
<br>

The input prompt is parsed based on ```;``` first. The resulting multiple commands are checked if they have loop statements. Then the commands the parsed based on redirection pipes.
<br>

The ```stringSplitter``` function takes a command, delimits the prompt based on all the characters in the delimit string and stores as individual entries into a 2D character array.
<br>

The ```actionHandler``` function takes the input prompt after being split based on ```;```, loop, redirection and pipe detections. If redirection, a new file is created and the file descriptors are stored. If pipe, two sets of pipes are created and the input command is split based on ```|```
<br>

The resulting command for performing the respective built-in operations, else a new process is forked and the operation is performed with an ```execv``` call. If pipes, the forking operation happens in a loop to handle information.