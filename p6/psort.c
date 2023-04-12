/**
 * @file psort.c
 * @author Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
 * @author Vishal Naik (vvnaik2@wisc.edu)
 * @brief Parallel Sort to leverage multi-processors
 * @version 2.0
 * @date 2023-04-11
 * @copyend Copyend (c) 2023
 */

/*
Ref: https://www.prowaretech.com/articles/current/c-plus-plus/algorithms/merge-sort-parallel#!
Ref: https://www.geeksforgeeks.org/merge-sort-using-multi-threading/
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
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct{
    int key;
    char *rec;
} records;

typedef struct {
    records *input;
    int threads;
    uint entries;
} threadArgs;

typedef struct {
    int start;
    int end;
} threadData;

int id = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Function to merge lists based on input indexes
 * 
 * @param input pointer to struct records that holds pointer to the actual records read
 * @param start starting index of the records to merge
 * @param mid middle index of the records to merge
 * @param end end index of the records to merge
 */
void merge(records *input, int start, int mid, int end) {
    int i, j, k;
    int leftHalfElements = mid - start + 1;
    int rightHalfElements = end - mid;

    records *leftHalfArr = (records *)malloc(leftHalfElements * sizeof(records));
    records *rightHalfArr = (records *)malloc(rightHalfElements * sizeof(records));

    for (i = 0; i < leftHalfElements; i++)
        leftHalfArr[i] = input[start + i];
    for (i = 0; i < rightHalfElements; i++)
        rightHalfArr[i] = input[mid + i + 1];

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
    free (leftHalfArr);
    free (rightHalfArr);
}

/**
 * @brief Function to recursively split the list and merge
 * 
 * @param input pointer to struct records that holds pointer to the actual records read
 * @param start starting index of the records to merge
 * @param end end index of the records to merge
 */
void mergeSort(records *input, int start, int end) {
    if (start < end) {
        int mid = start + (end - start) / 2;

        mergeSort(input, start, mid);
        mergeSort(input, mid + 1, end);

        merge(input, start, mid, end);
    }
}

/**
 * @brief Function to extract the arguments from the thread structure
 * 
 * @param args input argument for each thread
 */
void mergeHelper(void *args) {
    threadArgs *dummy = (threadArgs*) args;
    int thread = id++;
    int start = thread * (dummy->entries / dummy->threads);
    int end = (thread + 1) * (dummy->entries / dummy->threads) - 1;
    if(thread == dummy->threads - 1)
        end = dummy->entries - 1;
    int mid = start + (end - start) / 2;
    if (start < end) {
        mergeSort(dummy->input,start, mid);
        mergeSort(dummy->input,mid + 1, end);
        merge(dummy->input,start, mid, end);
    }
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
        #if debug
            printf("Number of threads cannot be less than 1\n");
        #endif
        return 0;
    }
    #if debug
        printf("Number of threads: %d\n", numOfThreads);
    #endif

    char *inputFile = argv[1];
    #if debug
        printf("Input File: %s\n", inputFile);
    #endif

    int inputFD;
    if((inputFD = open(inputFile, O_RDWR)) == -1) {
        #if debug
            printf("Input open failed\n");
        #endif
        return 0;
    }

    struct stat inputFileStats;
    fstat(inputFD, &inputFileStats);
    const uint entries = inputFileStats.st_size / RECORD_SIZE;
    #if debug
        printf("Number of entries in the input file: %d\n", entries);
    #endif

    char * inputRecords = (char *)mmap(NULL, inputFileStats.st_size, PROT_READ, MAP_SHARED, inputFD, 0);
    close(inputFD);

    records *duplicateRecords = (records *)malloc(entries * sizeof(records));

    /*
        Inspired from rcheck.c
    */
    records *c = duplicateRecords;
    for (char *r = inputRecords; r < inputRecords + entries * 100; r += 100) {
        c->key = *(int *)r;
        c->rec = r;
        c++;
    }

    char *outputFile = argv[2];
    #if debug
        printf("Output File: %s\n", outputFile);
    #endif
    
    int outputFD;
    if((outputFD = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
        #if debug
            printf("Output open failed\n");
        #endif
        return 0;
    }

    threadArgs arguments;
    pthread_t threads[numOfThreads];
    threadData* threadList = (threadData*)malloc(sizeof(threadData) * numOfThreads);
    int mid = entries / numOfThreads;

    clock_t startTime, endTime;

    pthread_mutex_init(&lock, NULL);

    startTime = clock();
    arguments.threads = numOfThreads;
    arguments.entries = entries;
    for(uint i = 0; i < numOfThreads; i++) {
        threadList[i].start = i * mid;
        threadList[i].end = threadList[i].start + mid - 1;

        if(i == numOfThreads - 1)
            threadList[i].end = entries - 1;

        arguments.input = duplicateRecords;
        pthread_create(&threads[i], NULL, (void *)mergeHelper, (void *)&arguments);
    }

    for(uint i = 0; i < numOfThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    for(uint i = 1; i < numOfThreads; i++) {
        merge(duplicateRecords, threadList[0].start, threadList[i].start - 1, threadList[i].end);
    }
    endTime = clock();

    for(uint i = 0; i < entries; i++) {
        if(write(outputFD, (void *)duplicateRecords[i].rec, RECORD_SIZE) == -1) {
            #if debug
                printf("Write to output file failed\n");
            #endif
        }
    }

    printf("Time taken: %lf\n", (endTime - startTime) / (double)CLOCKS_PER_SEC);

    munmap((void *)inputRecords, inputFileStats.st_size);
    fflush(NULL);
    close(outputFD);

    return 0;
}