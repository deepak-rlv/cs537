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

int loopIter = 1;
int pipesFD[2][2];
const char error_message[30] = "An error has occurred\n";

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

int actionHandler(char * singleCommand, char * ifPipe, char * ifRedirect){
    int stdOutBackup = dup(STDOUT_FILENO);
    int stdErrBackup = dup(STDERR_FILENO);
    int numOfCommandsInRedirection, outFileHandler, errFileHandler;
    char **redirectionCommands = malloc(sizeof(char*) * (strlen(singleCommand) + 1));
    numOfCommandsInRedirection = stringSplitter(redirectionCommands, singleCommand, ">");
    if(ifRedirect){
        while(isspace(*redirectionCommands[numOfCommandsInRedirection - 1])){
            redirectionCommands[numOfCommandsInRedirection - 1] ++;
        }
        redirectionCommands[numOfCommandsInRedirection - 1][strcspn(redirectionCommands[numOfCommandsInRedirection - 1], " \r\n\t")]='\0';
        outFileHandler = open(redirectionCommands[numOfCommandsInRedirection - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
        errFileHandler = open(redirectionCommands[numOfCommandsInRedirection - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
        if(!ifPipe){
            dup2(outFileHandler, STDOUT_FILENO);
            dup2(errFileHandler, STDERR_FILENO);
        }
    }

    char **args = malloc(sizeof(char*) * (strlen(redirectionCommands[0]) + 1));

    int numOfArgs;
    if(ifPipe){
        numOfArgs = stringSplitter(args, redirectionCommands[0], "|");
        if((pipe(pipesFD[0]) < 0) || (pipe(pipesFD[1]) < 0)){
            #if debug
                printf("Pipe creation failed\n");
            #endif
            write(STDERR_FILENO, error_message, strlen(error_message));
            free(args);
            return 0;
        }
    }
    else
        numOfArgs = stringSplitter(args, redirectionCommands[0], " \n\t\r");

    if(!strcmp(args[0],"exit")){
        exit(0);
    } else if(!strcmp(args[0], "cd")){
        if(numOfArgs > 2 || numOfArgs == 1){
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
        int * forkReturnPid = malloc(sizeof(int) * numOfArgs);
        for(int pipeIter = 0; pipeIter < numOfArgs; pipeIter++){
            forkReturnPid[pipeIter] = fork();
            if(forkReturnPid[pipeIter] < 0){
                #if debug
                    printf("Fork failed\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(args);
                return 0;
            } else if(forkReturnPid[pipeIter] == 0){
                char **pipeArgs = malloc(sizeof(char*) * (strlen(args[pipeIter]) + 1));
                if(ifPipe){
                    if(pipeIter == 0)
                        dup2(pipesFD[0][1], STDOUT_FILENO);
                    else if(pipeIter == numOfArgs - 1 && numOfArgs != 2){
                        dup2(pipesFD[1][0], STDIN_FILENO);
                        if(ifRedirect)
			                dup2(outFileHandler, STDOUT_FILENO);
                    }
                    else if (numOfArgs == 2){
                        dup2(pipesFD[0][0], STDIN_FILENO);
                        if(ifRedirect)
                            dup2(outFileHandler, STDOUT_FILENO);
                    } else{
                        dup2(pipesFD[0][0], STDIN_FILENO);
                        dup2(pipesFD[1][1], STDOUT_FILENO);
                    }
                    close(pipesFD[0][0]);
                    close(pipesFD[0][1]);
                    close(pipesFD[1][0]);
                    close(pipesFD[1][1]);

                    stringSplitter(pipeArgs, args[pipeIter], " \n\t\r") ;

                }
                int execReturn = 0;
                if(!ifPipe)
                    execReturn = execv(args[0], args);
                else
                    execReturn = execv(pipeArgs[0], pipeArgs);
                if (execReturn < 0){
                    #if debug
                        printf("Exec failed.\n");
                    #endif

                    if(ifRedirect){
                        //executable not found. Do not redirect to file
                        dup2(stdErrBackup, STDERR_FILENO);
                    }
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    fflush(stderr);
                    free(args);
                    exit(0);
                }
            }
            if(!ifPipe)
                break;
        }
        if(ifPipe){
            close(pipesFD[0][1]);
            close(pipesFD[0][0]);
            close(pipesFD[1][1]);
            close(pipesFD[1][0]);
        }

        int waitReturn = 0;
        for(int i = 0; i < numOfArgs; i++){
            waitpid(forkReturnPid[i], NULL, 0);
            if(waitReturn < 0){
                #if debug
                    printf("Wait failed\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(forkReturnPid);
                free(args);
                return 0;
            }
        }
        free(forkReturnPid);
    }
	if(ifRedirect){
        fflush(stdout);
        fflush(stderr);
        close(outFileHandler);
        close(errFileHandler);
        dup2(stdOutBackup, STDOUT_FILENO);
        dup2(stdErrBackup, STDERR_FILENO);
    }
    free(args);
    free(redirectionCommands);
    return 0;
}

int redirectHandler(/* char * redirectPointer, */ char * prompt){
    char **multipleCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
    int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

    for(int i = 0; i < numberOfCommands; i++){
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
    }
    free(multipleCommands);
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
if(numOfCommands<3){
        #if debug
            printf("Loop iterations not specified\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        free(individualCommands);
        return 1;
    }
    for(int i=0; i<strlen(individualCommands[1]);i++){
        if(!isdigit((individualCommands[1][i]))){
            x=1;
            break;
        }
    }
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

        #if debug
            printf("Input Command: %s\n",prompt);
        #endif

        char * dummy = strdup(prompt);
        char * ifRedirect = strchr(prompt,'>');
        char * ifPipe = strchr(prompt,'|');
            if(ifRedirect)
                if(redirectHandler(dummy))
                    continue;
            if(ifPipe)
                if(pipeHandler(ifPipe, dummy))
                    continue;

        char **multipleCommands = malloc(sizeof(char*) * (strlen(prompt) + 1));
        int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");
        
        for(int i = 0; i < numberOfCommands; i++){
            char **individualCommands = malloc(sizeof(char*) * (strlen(multipleCommands[i]) + 1));
            ifRedirect = strchr(multipleCommands[i],'>');
            ifPipe = strchr(multipleCommands[i],'|');
            int ifLoop = 0;

            //could have used char * ifLoop = strstr(multipleCommands[i], "loop");
            //but if file name or other executables named loops exists, then there will be a wrong detection
            if( multipleCommands[i][0] == 'l' &&
                multipleCommands[i][1] == 'o' &&
                multipleCommands[i][2] == 'o' &&
                multipleCommands[i][3] == 'p'
            )
                ifLoop = 1;

            if(!ifLoop)
                loopIter = 1;
            /* char * dummy = strdup(multipleCommands[i]);
            if(ifRedirect)
                if(redirectHandler(ifRedirect, dummy))
                    continue;
            if(ifPipe)
                if(pipeHandler(ifPipe, dummy))
                    continue; */
            if(ifLoop){
                char loopIterStr[11] = "\0"; //since int max is 10 digits
                if(loopHandler(multipleCommands[i], dummy, loopIterStr))
                    continue;
                multipleCommands[i]+=(6 + strlen(loopIterStr));
            }
            
            for(int looping =0;looping<loopIter;looping++){
                char * loopCommand = strdup(multipleCommands[i]);

            if(whiteSpaceCommand(multipleCommands[i]))
                continue;
            // if(ifRedirect && ifPipe){
                actionHandler(multipleCommands[i], ifPipe, ifRedirect);
            // } 
            /* else if(ifPipe){
                if((pipe(pipesFD[0]) < 0) || (pipe(pipesFD[1]) < 0)){
                    #if debug
                        printf("Pipe creation failed\n");
                    #endif
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    break;
                }
                int numOfArgs = stringSplitter(individualCommands, multipleCommands[i], "|");
                for(pipeIter = 0; pipeIter < numOfArgs; pipeIter++){
                    actionHandler(multipleCommands[i], ifPipe);
                }
            } */
            
            
            
            //  {
            //     int numOfCommands = stringSplitter(individualCommands, multipleCommands[i], ">");
            //     while(isspace(*individualCommands[numOfCommands-1])){
            //         individualCommands[numOfCommands-1] ++;
            //     }
            //     // individualCommands[numOfCommands - 1][strlen(individualCommands[numOfCommands - 1])-1]='\0';
            //     individualCommands[numOfCommands - 1][strcspn(individualCommands[numOfCommands - 1], " \r\n\t")]='\0';
            //     int outFileHandler = open(individualCommands[numOfCommands - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
            //     int errFileHandler = open(individualCommands[numOfCommands - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
            //     dup2(outFileHandler, fileno(stdout));
            //     dup2(errFileHandler, fileno(stderr));
            //     actionHandler(multipleCommands[i], NULL);
            //     fflush(stdout); 
            //     fflush(stderr); 
            //     close(outFileHandler);
            //     close(errFileHandler);
            //     dup2(stdOutBackup, fileno(stdout));
            //     dup2(stdErrBackup, fileno(stderr));
            // }
            strcpy(multipleCommands[i],loopCommand);
            free(loopCommand);
            }
            free(individualCommands);
        }
        /* 
        close(stdOutBackup);
        close(stdErrBackup); */
        free(dummy);
        free(multipleCommands);
    }
    return 0;
}
