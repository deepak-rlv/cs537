/**
 * @file psort.c
 * @author Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
 * @author Vishal Naik (vvnaik2@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*
    Set flag to 1 to view debug information
    By default, set to 0
*/
#ifndef debug
    #define debug 0
#endif

#define ARGUMENTS 3
#define RECORD_SIZE 100
#define ARRAY_SIZE RECORD_SIZE + 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    if(argc - 1 != ARGUMENTS){
        printf("!!! Not enough arguments !!!\n\n"
        "Usage: ./psort input output 4\n"
        "input\t\tThe input file to read records for sort\n"
        "output\t\tThe output file where records will be written after sort\n"
        "numThreads\tNumber of threads that shall perform the sort operation\n");

        return 0;
    }

    /*
        The number of threads for a process cannot be less than 1
    */
    int threads = atoi(argv[3]);
    if(threads < 1){
        #ifdef debug
            printf("Number of threads cannot be less than 1\n");
        #endif
        return 0;
    }
    #ifdef debug
        printf("Number of threads: %d\n", threads);
    #endif

    char *inputFile = argv[1];
    char *outputFile = argv[2];
    #ifdef debug
        printf("Input File: %s\nOutput File: %s\n", inputFile, outputFile);
    #endif

    FILE *inputFP;
    if((inputFP = fopen(inputFile, "r")) != 0){
        #ifdef debug
            printf("Input fopen failed\n");
        #endif
        return 0;
    }

    FILE *outputFP;
    if((outputFP = fopen(outputFile, "w")) != 0){
        #ifdef debug
            printf("Output fopen failed\n");
        #endif
        return 0;
    }

    // char *records[ARRAY_SIZE] = fgets( char *buf, int n, FILE *fp );

    return 0;
}