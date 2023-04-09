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

typedef struct{
    int key;
    char *val;
} stream;

void merge(char *arr, int l, int m, int r)
{
    int i, j, k;
    int n1 = (m - l)/100 + 1;
    int n2 = (r - m)/100;

    char L[n1][RECORD_SIZE], R[n2][RECORD_SIZE];

    for (i = 0; i < n1; i++)
        memcpy(L[i], arr + l + i * 100, RECORD_SIZE);
    for (j = 0; j < n2; j++)
        memcpy(R[j], arr + m + 100 + j * 100, RECORD_SIZE);

    i = 0;
    j = 0;
    k = l / 100;
    while (i < n1 && j < n2) {
        int key1 = *(int*)L[i];
        int key2 = *(int*)R[j];
        if (key1 <= key2) {
            memcpy(arr + k * 100, L[i], RECORD_SIZE);
            i++;
        }
        else {
            memcpy(arr + k * 100, R[j], RECORD_SIZE);
            j++;
        }
        k++;
    }

    while (i < n1) {
        memcpy(arr + k * 100, L[i], RECORD_SIZE);
        i++;
        k++;
    }

    while (j < n2) {
        memcpy(arr + k * 100, R[j], RECORD_SIZE);
        j++;
        k++;
    }
}
 

void mergeSort(char *inputMMAP, int l, int r)
{
    if (l < r) {
        int m = ((l + (r - l) / 2 ) / 100 ) * 100;

        mergeSort(inputMMAP, l, m);
        mergeSort(inputMMAP, m + 100, r);

        merge(inputMMAP, l, m, r);
    }
}

struct args {
    char *input;
    int start, end;
};

void mergeHelper(void *tArgs){
    struct args *arg = (struct args*) tArgs;
    mergeSort(arg->input, arg->start, arg->end);
}

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
    int numOfThreads = atoi(argv[3]);
    if(numOfThreads < MIN_THREADS){
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
    if((inputFD = open(inputFile, O_RDWR)) == -1){
        #ifdef debug
            printf("Input open failed\n");
        #endif
        return 0;
    }

    int outputFD;
    if((outputFD = open(outputFile, O_CREAT | O_WRONLY, 0600)) == -1){
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

    struct args tArgs;
    int mid = entries / numOfThreads;

    char * inputRecords = (char *)mmap(NULL, inputFileStats.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, inputFD, 0);
    close(inputFD);
    pthread_t threads[numOfThreads];

    for(int i = 0; i < numOfThreads; i++){
        tArgs.start = i * mid * RECORD_SIZE;
        tArgs.end = tArgs.start + (mid - 1) * RECORD_SIZE;
        tArgs.input = inputRecords;
        pthread_create(&threads[i], NULL, (void *)mergeHelper, (void *)&tArgs);   
    }

    for(int i = 0; i < numOfThreads; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}