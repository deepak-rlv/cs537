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
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

const char error_message[30] = "An error has occurred\n";
int loopIter=1;

int stringSplitter(char **multipleCommands, char * prompt, char * delim){
    char * command = strtok(prompt, delim);
    int i=0;
    while (command != NULL){
        multipleCommands[i] = command;
        command = strtok(NULL, delim);
        i++;
    }
    multipleCommands[i] = NULL;
    free(command);
    return i;
}

int actionHandler(char * singleCommand){
    char **args = malloc(sizeof(char*) * (strlen(singleCommand) + 1));


    int numOfArgs = stringSplitter(args, singleCommand, " \n\t\r") - 1;

        if(!strcmp(args[0],"exit")){
            exit(0);
        } else if(!strcmp(args[0], "cd")){
            if(numOfArgs > 1 || numOfArgs == 0){
                #if debug
                    printf("cd arguments error\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message)); 
            } else {
                int chdirReturn = chdir(args[1]);
                if(chdirReturn < 0){
                    #if debug
                        printf("chdir error\n");
                    #endif
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
        } else if(!strcmp(args[0], "pwd")){
            char currentDir[PWD_SIZE];
            if(!getcwd(currentDir, PWD_SIZE));
            printf("%s\n", currentDir);
        } else {
            int forkReturn = fork();
            if(forkReturn < 0){
                #if debug
                    printf("Fork failed\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
            } else if(forkReturn == 0){
                int execReturn = execv(args[0], args);
                if (execReturn < 0){
                    #if debug
                        printf("Exec failed.\n");
                    #endif

                    if(errno == EACCES){
                        //executable not found. Do not redirect to file
                    }
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
                exit(0);
            } else {
                int waitReturn = wait(NULL);
                if(waitReturn < 0){
                    #if debug
                        printf("Wait failed\n");
                    #endif
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
        }
    
    free(args);
    return 0;
}

int redirectHandler(char * redirectPointer, char * prompt){
    /* char **multipleCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
    int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

    for(int i = 0; i < numberOfCommands; i++){ */
        char **individualCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
        int numOfCommands = stringSplitter(individualCommands, prompt, ">");

        if(numOfCommands == 1){
            #if debug
                printf("No command to redirect\n");
            #endif
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            free(individualCommands);
            return 1;
        }
        free(individualCommands);
    /* }
    free(multipleCommands); */
    return 0;
}

int pipeHandler(char * pipePointer, char * prompt){
    /* char **multipleCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
    int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

    for(int i = 0; i < numberOfCommands; i++){ */
        char **individualCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
        int numOfCommands = stringSplitter(individualCommands, prompt, "|");

        if(numOfCommands == 1){
            #if debug
                printf("No command to pipe\n");
            #endif
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            free(individualCommands);
            return 1;
        }
        free(individualCommands);
    /* }
    free(multipleCommands); */
    return 0;
}

int loopHandler(char *loopPointer, char * prompt, char * loopIterStr){
    char **individualCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
    int numOfCommands = stringSplitter(individualCommands, prompt, " \n\r\t");
    int x=0;
    for(int i=0; i<strlen(individualCommands[1]);i++){
        if(!isdigit((individualCommands[1][i]))){
            x=1;
            break;
        }
    }
    if(numOfCommands < 3) //should fail for loop & loop <num> cases
        x = 1;
    if(x){
        #if debug
            printf("Loop iterations not specified\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        free(individualCommands);
        return 1;
    }
    loopIter = atoi(individualCommands[1]);
    free(individualCommands);
    return 0;
}

int whiteSpaceCommand(char * prompt){
    int i = 0;
    while(isspace(prompt[i])){
        i++;
    }
    if(i == strlen(prompt))
        return 1;
    return 0;
}

//ToDo check if number of iterations is provided with loops

int main(int argc, char *argv[]){
    if(argc > 1){
        #if debug
            printf("No arguments accepted. Exiting.\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return 1;
    }
    
    size_t promptSize = 20;
    char *prompt = malloc(sizeof(char) * promptSize);
    
    int charactersRead;
    while(1){

        do{
            printf("smash> ");
            fflush(stdout);
            charactersRead = getline(&prompt, &promptSize, stdin);
            if(charactersRead == -1){
                #if debug
                    printf("Failed to read command.\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 0;
            }
        }while(whiteSpaceCommand(prompt));
        

        // prompt[charactersRead - 1] = '\0';   // Replacing the \n at the end of the prompt with \0

        #if debug
            printf("Input Command: %s\n",prompt);
        #endif

        char **multipleCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));

        /* char * ifRedirect = strchr(prompt,'>');
        char * ifPipe = strchr(prompt,'|');
        char * dummy = strdup(prompt);
        if(ifRedirect)
            if(redirectHandler(ifRedirect, dummy))
                continue;
        if(ifPipe)
            if(pipeHandler(ifPipe, dummy))
                continue;
        free(dummy); */

        int stdOutBackup = dup(fileno(stdout));
        int stdErrBackup = dup(fileno(stderr));

        int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");
        
        for(int i = 0; i < numberOfCommands; i++){
            char **individualCommands = malloc(sizeof(char*) * (strlen(multipleCommands[i]) + 1));
            char * ifRedirect = strchr(multipleCommands[i],'>');
            char * ifPipe = strchr(multipleCommands[i],'|');
            int ifLoop = 0;

            //could have used char * ifLoop = strstr(multipleCommands[i], "loop");
            //but if file name or other executables named loops exists, then there will be a wrong detection
            if( multipleCommands[i][0] == 'l' &&
                multipleCommands[i][1] == 'o' &&
                multipleCommands[i][2] == 'o' &&
                multipleCommands[i][3] == 'p'
            )
                ifLoop = 1;
            char * dummy = strdup(multipleCommands[i]);
            if(ifRedirect)
                if(redirectHandler(ifRedirect, dummy))
                    continue;
            if(ifPipe)
                if(pipeHandler(ifPipe, dummy))
                    continue;
            if(ifLoop){
                char loopIterStr[11] = "\0"; //since int max is 10 digits
                if(loopHandler(multipleCommands[i], dummy, loopIterStr))
                    continue;
                multipleCommands[i]+=(6 + strlen(loopIterStr));
            }
            free(dummy);
            
            for(int looping =0;looping<loopIter;looping++){
                char * loopCommand = strdup(multipleCommands[i]);

            if(whiteSpaceCommand(multipleCommands[i]))
                continue;
            if(!ifRedirect && !ifPipe){
                actionHandler(multipleCommands[i]);
            } else {
                int numOfCommands = stringSplitter(individualCommands, multipleCommands[i], ">");
                while(isspace(*individualCommands[numOfCommands-1])){
                    individualCommands[numOfCommands-1] ++;
                }
                // individualCommands[numOfCommands - 1][strlen(individualCommands[numOfCommands - 1])-1]='\0';
                individualCommands[numOfCommands - 1][strcspn(individualCommands[numOfCommands - 1], " \r\n\t")]='\0';
                int outFileHandler = open(individualCommands[numOfCommands - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
                int errFileHandler = open(individualCommands[numOfCommands - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
                dup2(outFileHandler, fileno(stdout));
                dup2(errFileHandler, fileno(stderr));
                actionHandler(multipleCommands[i]);
                fflush(stdout); 
                fflush(stderr); 
                close(outFileHandler);
                close(errFileHandler);
                dup2(stdOutBackup, fileno(stdout));
                dup2(stdErrBackup, fileno(stderr));
            }
            strcpy(multipleCommands[i],loopCommand);
            free(loopCommand);
            }
            free(individualCommands);
        }
        
        close(stdOutBackup);
        close(stdErrBackup);
        free(multipleCommands);
    }
    return 0;
}
