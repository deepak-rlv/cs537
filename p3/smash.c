/**
 * @file smash.c
 * @author Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
 * @brief Unix Shell
 * @version 0.1
 * @date 2023-02-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef debug
    #define debug 0
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int actionHandler(char * path, ...){
    char * argumentsPtr = (char *)&path + 1;
    int forkReturn = fork();
    if(forkReturn < 0){
        #if debug
            printf("Fork failed\n");
        #endif
        return 1;
    } else if(forkReturn == 0){
        execv("/bin",path);
    } else {
        int waitReturn = wait(NULL);
    }
    return 0;
}

int main(int argc, char *argv[]){
    if(argc > 1){
        #if debug
            printf("No arguments accepted. Exiting.\n");
        #endif
        return 1;
    }

    size_t promptSize = 20;
    char *prompt = malloc(sizeof(char) * promptSize);
    
    int charactersRead;
    do{
        printf("smash> ");
        charactersRead = getline(&prompt, &promptSize, stdin);
        if(charactersRead == -1){
            #if debug
                printf("Failed to read command.\n");
            #endif
            return 1;
        }
        prompt[charactersRead - 1] = '\0';   // Replacing the \n at the end of the prompt with \0

        #if debug
            printf("Input Command: %s\n",prompt);
        #endif

        actionHandler("ls");

        /* switch(1){
            case 1   :   actionHandler("ls");
                            break;

            default     :   //printf("Command not found\n");
                            break;
        } */

    }while(strcmp(prompt,"exit"));

    exit(0);

    return 0;
}