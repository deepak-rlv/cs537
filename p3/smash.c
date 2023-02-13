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

#define PWD_SIZE 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int stringSplitter(char **multipleCommands, char * prompt, char * delim){
    char * command = strtok(prompt, delim);
    int i=0;
    while (command != NULL){
        multipleCommands[i] = command;
        command = strtok(NULL, delim);
        i++;
    }
    return i;
}

int actionHandler(char * singleCommand){
    char * original = strdup(singleCommand);
    char **args = malloc(sizeof(char) * (strlen(singleCommand) + 1));

    int numOfArgs = stringSplitter(args, singleCommand, " \n\t") - 1;


    if(!strcmp(args[0],"exit")){
        exit(0);
    } else if(!strcmp(args[0], "cd")){
        if(numOfArgs > 1 || numOfArgs == 0){
            #if debug
                printf("cd arguments error\n");
            #endif
        } else {
            int chdirReturn = chdir(args[1]);
            if(chdirReturn < 0){
                #if debug
                    printf("chdir error\n");
                #endif
            }
        }
    } else if(!strcmp(args[0], "pwd")){
        char currentDir[PWD_SIZE];
        if(!getcwd(currentDir, PWD_SIZE));
        printf("%s\n", currentDir);
    } else if(!strcmp(args[0], "loop")){
        int interations = atoi(args[1]);
        char *loopArgs = strstr(original, args[2]);
        free(args);
        for(int i = 0; i < interations; i++){
            actionHandler(loopArgs);
        }
        return 1;
    } else {
        int forkReturn = fork();
        if(forkReturn < 0){
            #if debug
                printf("Fork failed\n");
            #endif
            free(args);
            return 1;
        } else if(forkReturn == 0){
            int execReturn = execvp(args[0], args);
            if (execReturn < 0){
                #if debug
                    printf("Exec failed.\n");
                #endif
                exit(0);
            }
            exit(0);
        } else {
            int waitReturn = wait(NULL);
            if(waitReturn < 0){
                #if debug
                    printf("Wait failed\n");
                #endif
                free(args);
                return 1;
            }
        }
    }
    free(args);
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
    while(1){
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

        char **multipleCommands = malloc(sizeof(char) * (strlen(prompt) + 1));
        int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

        for(int i = 0; i < numberOfCommands; i++){
            char **individualCommands = malloc(sizeof(char) * (strlen(multipleCommands[i]) + 1));
            char * ifRedirect = strchr(multipleCommands[i], '>');
            if(ifRedirect == NULL)
                actionHandler(multipleCommands[i]);
            else{
                int numOfCommands = stringSplitter(individualCommands, multipleCommands[i], ">");
                freopen(individualCommands[1], "a", stdout);
                actionHandler(individualCommands[0]);
                fflush(stdout);
            }
            free(individualCommands);
        }
        
        free(multipleCommands);
    }
    return 0;
}