/**
 * @file psort.c
 * @author Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
 * @author Vishal Naik (vvnaik2@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-06
 * 
 * @copyend Copyend (c) 2023
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
#define MIN_THREADS 1
#define RECORD_SIZE 100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <errno.h>

typedef struct{
    int key;
    char val[RECORD_SIZE - 4];
} record;

typedef struct {
    record *input;
    int start, end;
} threadArgs;

void merge(record *input, int start, int mid, int end) {
    int i, j, k;
    int leftHalfElements = mid - start + 1;
    int rightHalfElements = end - mid;

    record leftHalfArr[leftHalfElements], rightHalfArr[rightHalfElements];

    for (i = 0; i < leftHalfElements; i++)
        leftHalfArr[i] = input[start + i];
    for (j = 0; j < rightHalfElements; j++)
        rightHalfArr[j] = input[mid + j + 1];

    i = 0;
    j = 0;
    k = start;

    while (i < leftHalfElements && j < rightHalfElements) {
        if (leftHalfArr[i].key <= rightHalfArr[j].key) {
            input[k] = leftHalfArr[i];
            i++;
        }
        else {
            input[k] = rightHalfArr[j];
            j++;
        }
        k++;
    }

    while (i < leftHalfElements) {
        input[k] = leftHalfArr[i];
        i++;
        k++;
    }

    while (j < rightHalfElements) {
        input[k] = rightHalfArr[j];
        j++;
        k++;
    }
}
 

void mergeSort(record *inputMMAP, int start, int end) {
    if (start < end) {
        int mid = start + (end - start) / 2;

        mergeSort(inputMMAP, start, mid);
        mergeSort(inputMMAP, mid + 1, end);

        merge(inputMMAP, start, mid, end);
    }
}

void mergeHelper(void *args) {
    threadArgs *dummy = (threadArgs*) args;
    mergeSort(dummy->input, dummy->start, dummy->end);
}

int main(int argc, char *argv[]) {
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
    int numOfThreads = atoi(argv[3]);
    if(numOfThreads < MIN_THREADS) {
        #ifdef debug
            printf("Number of threads cannot be less than 1\n");
        #endif
        return 0;
    }
    #ifdef debug
        printf("Number of threads: %d\n", numOfThreads);
    #endif

    char *inputFile = argv[1];
    char *outputFile = argv[2];
    #ifdef debug
        printf("Input File: %s\nOutput File: %s\n", inputFile, outputFile);
    #endif

    int inputFD;
    if((inputFD = open(inputFile, O_RDWR)) == -1) {
        #ifdef debug
            printf("Input open failed\n");
        #endif
        return 0;
    }

    int outputFD;
    if((outputFD = open(outputFile, O_CREAT | O_WRONLY, 0600)) == -1) {
        #ifdef debug
            printf("Output open failed\n");
        #endif
        return 0;
    }

    struct stat inputFileStats;
    fstat(inputFD, &inputFileStats);
    const int entries = inputFileStats.st_size / RECORD_SIZE;
    #ifdef debug
        printf("Number of entries in the input file: %d\n", entries);
    #endif

    threadArgs arguments;
    int mid = entries / numOfThreads;

    record backup[entries];

    char * inputRecords = (char *)mmap(NULL, inputFileStats.st_size, PROT_READ, MAP_SHARED, inputFD, 0);
    close(inputFD);

    for(int i = 0; i < entries; i++) {
        backup[i].key = *(int *)(inputRecords + i * RECORD_SIZE);
        memcpy(backup[i].val, inputRecords + 4 + i * RECORD_SIZE, RECORD_SIZE - 4);
    }
    munmap((void *)inputRecords, inputFileStats.st_size);

    pthread_t threads[numOfThreads];

    for(int i = 0; i < numOfThreads; i++) {
        arguments.start = i * mid;
        arguments.end = arguments.start + mid - 1;
        arguments.input = backup;
        pthread_create(&threads[i], NULL, (void *)mergeHelper, (void *)&arguments);   
    }

    for(int i = 0; i < numOfThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    for(int i = 0; i < entries; i++) {
        write(outputFD, (void *)&backup[i], RECORD_SIZE);
    }
    fflush(NULL);
    close(outputFD);

    return 0;
}