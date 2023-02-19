/**
 * @file smash.c
 * @author Deepak Charan Logavaseekaran (logavaseekar@wisc.edu)
 * @brief Unix Shell
 * @version 2.0
 * @date 2023-02-19
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

/*
    Maximum size of the string representing
    the present working directory
*/
#define PWD_SIZE 200

/*
    Variable holding the looping iterations for a loop command
*/
int loopIter = 1;

/*
    int** to hold the fileDescriptors for piping commands
    Array of 2 entries for piping between 2 programs (since pipes are unidirectional)
    The other 2 are used to extend this to pipe chaining in a loop
    Ref: https://code-vault.net/course/46qpfr4tkz:1603732431896/lesson/oxyoxbvnak:1603732432935
*/
int pipesFD[2][2];

/*
    Error string to be displayed for all errors detected by the shell
*/
const char error_message[30] = "An error has occurred\n";

/**
 * @brief Split the input string based on all characters in a delimiter string
 *        and stored as a 2D char array
 * 
 * @param multipleCommands Resulting array of strings split based on the delimiting string
 * @param prompt String to be split
 * @param delim String of all delimiting characters
 * @return int The number of resultant strings 
 */
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

/**
 * @brief Executes the commands, handles redirection and pipes
 * 
 * @param singleCommand Command to execute
 * @param ifPipe Pointer to indicate if piping need to be done
 * @param ifRedirect Pointer to indicate if redirection need to be done
 * @return int -1 if nested loops detected, 0 otherwise
 */
int actionHandler(char * singleCommand, char * ifPipe, char * ifRedirect){
    /*
        Taking backups of stdout & stderr to recover after redirection
    */
    int stdOutBackup = dup(STDOUT_FILENO);
    int stdErrBackup = dup(STDERR_FILENO);

    /*
        Splitting commands based on > to handle redirection if ifRedirect is not NULL
    */
    int numOfCommandsInRedirection, outFileHandler, errFileHandler;
    char **redirectionCommands = (char **)malloc(sizeof(char *) * (strlen(singleCommand) + 1));
    numOfCommandsInRedirection = stringSplitter(redirectionCommands, singleCommand, ">");
    if(ifRedirect){
        while(isspace(*redirectionCommands[numOfCommandsInRedirection - 1])){
            redirectionCommands[numOfCommandsInRedirection - 1] ++;
        }
        /*
            To avoid trailing whitespace in file names, the last character is forced to \0
        */
        redirectionCommands[numOfCommandsInRedirection - 1][strcspn(redirectionCommands[numOfCommandsInRedirection - 1], " \r\n\t")]='\0';
        outFileHandler = open(redirectionCommands[numOfCommandsInRedirection - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
        errFileHandler = open(redirectionCommands[numOfCommandsInRedirection - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
        if(!ifPipe){
            dup2(outFileHandler, STDOUT_FILENO);
            dup2(errFileHandler, STDERR_FILENO);
        }
    }


    int numOfArgs;
    char **args = (char **)malloc(sizeof(char *) * (strlen(redirectionCommands[0]) + 1));
    /*
        Splitting commands based on | to handle pipes if ifPipe is not NULL
        else split based on white space and continue normal execution
    */
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

    /*
        Generating error if nested loops detected
    */
    if(!strcmp(args[0],"loop")){
        #if debug
            printf("Nested loops detected\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message));
        free(redirectionCommands);
        free(args);
        return -1;
    }

    if(!strcmp(args[0],"exit")){
        /*
            EXIT
        */
        /*
            Generating error if exit is passed with an argument
            Else, exit from shell
        */
        if(numOfArgs != 1){
            #if debug
                printf("Exit does not accept arguments\n");
            #endif
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        } else{
            exit(0);
        }
    } else if(!strcmp(args[0], "cd")){
        /*
            CD
        */
        /*
            cd always takes one argument
            generating errors if it does not
        */
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
        /*
            PWD
        */
        char currentDir[PWD_SIZE];
        if(!getcwd(currentDir, PWD_SIZE)){
            #if debug
                printf("Couldn't fetch current working directory\n");
            #endif
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        printf("%s\n", currentDir);
    } else {
        /*
            All other commands
        */

        /*
            Array of PIDs required in the case of piping commands
        */
        int * forkReturnPid = (int *)malloc(sizeof(int) * numOfArgs);
        memset(forkReturnPid, 0, sizeof(int) * numOfArgs);

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
                /*
                    CHILD PROCESS
                */
                char **pipeArgs = (char **)malloc(sizeof(char *) * (strlen(args[pipeIter]) + 1));
                if(ifPipe){
                    if(pipeIter == 0)
                        /*
                            Changing the stdout of process 1 to pipe to send to next process
                        */
                        dup2(pipesFD[0][1], STDOUT_FILENO);
                    else if(pipeIter == numOfArgs - 1 && numOfArgs != 2){
                        /*
                            If more than 1 pipe exists, change stdin of last process to pipe
                            to receive from previous process
                        */
                        dup2(pipesFD[1][0], STDIN_FILENO);

                        /*
                            In case of redirect, set the stdout of last process to output file
                        */
                        if(ifRedirect)
			                dup2(outFileHandler, STDOUT_FILENO);
                    }
                    else if (numOfArgs == 2){
                        /*
                            If only one pipe exits, change stdin of process 2 to pipe
                            to receive from process 1
                        */
                        dup2(pipesFD[0][0], STDIN_FILENO);

                        /*
                            In case of redirect, set the stdout of last process to output file
                        */
                        if(ifRedirect)
                            dup2(outFileHandler, STDOUT_FILENO);
                    } else{
                        /*
                            For all other cases, set stdin of process to pipe to receive input from
                            previous file and stdout of process to pipe to send to next process
                        */
                        dup2(pipesFD[0][0], STDIN_FILENO);
                        dup2(pipesFD[1][1], STDOUT_FILENO);
                    }
                    close(pipesFD[0][0]);
                    close(pipesFD[0][1]);
                    close(pipesFD[1][0]);
                    close(pipesFD[1][1]);

                    stringSplitter(pipeArgs, args[pipeIter], " \n\t\r");
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
                        /*
                            If executable not found, do not redirect error to file
                        */
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
        /*
            Parent waits till all child processes exit
        */
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

    /*
        Flushing all buffers, closing files and restoring stdout & stderr
    */
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

/**
 * @brief Finds errors related to the redirect command
 * 
 * @param prompt Input command to check
 * @return int 1 if error, 0 otherwise
 */
int redirectErrorHandler(char * prompt){
    char **multipleCommands = (char **)malloc(sizeof(char *) * (strlen(prompt) + 1));
    int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

    for(int i = 0; i < numberOfCommands; i++){
        char * ifRedirect = strchr(multipleCommands[i],'>');

        if(ifRedirect){
            char **individualCommands = (char **)malloc(sizeof(char *) * (strlen(multipleCommands[i]) + 1));
            int numOfCommands = stringSplitter(individualCommands, multipleCommands[i], "> \n\r\t");
            /*
                Error if no commands are found to redirect, ls > or > output
            */
            if(numOfCommands == 1){
                #if debug
                    printf("No command to redirect\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message)); 
                free(multipleCommands);
                free(individualCommands);
                return 1;
            }
            free(individualCommands);
        }
    }
    free(multipleCommands);
    return 0;
}

/**
 * @brief Finds errors related to the pipe command
 * 
 * @param prompt Input command to check
 * @return int 1 if error, 0 otherwise
 */
int pipeErrorHandler(char * prompt){
    char **multipleCommands = (char **)malloc(sizeof(char *) * (strlen(prompt) + 1));
    int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");

    for(int i = 0; i < numberOfCommands; i++){
        char * ifPipe = strchr(multipleCommands[i],'|');

        if(ifPipe){
            char **individualCommands = (char **)malloc(sizeof(char *) * (strlen(multipleCommands[i]) + 1));
            int numOfCommands = stringSplitter(individualCommands, multipleCommands[i], "| \n\r\t");
            /*
                Error if no commands are found to pipe, ls | or | ls
            */
            if(numOfCommands == 1){
                #if debug
                    printf("No command to pipe\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message)); 
                free(multipleCommands);
                free(individualCommands);
                return 1;
            }
            free(individualCommands);
        }
    }
    free(multipleCommands);
    return 0;
}

/**
 * @brief Finds errors related to the loop command
 * 
 * @param prompt Input command to check
 * @param loopIterStr Loop iterations in string (generated by this function)
 * @return int 1 if error, 0 otherwise
 */
int loopErrorHandler(char * prompt, char * loopIterStr){
    char **individualCommands = (char **)malloc(sizeof(char *) * (strlen(prompt) + 1));
    int numOfCommands = stringSplitter(individualCommands, prompt, " \n\r\t");
    int flag = 0;
    /*
        Error if command is only "loop "
    */
    if(numOfCommands < 2){
        #if debug
            printf("Only loop command found\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        free(individualCommands);
        return 1;
    }

    /*
        Error if loop iterations not specified "loop /bin/ls"
    */
    for(int i = 0; i < strlen(individualCommands[1]); i++){
        if(!isdigit((individualCommands[1][i]))){
            flag = 1;
            break;
        }
    }
    if(flag){
        #if debug
            printf("Loop iterations not specified\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        free(individualCommands);
        return 1;
    }

    /*
        Storing the loop iterations in the command to loopIter global var
    */
    loopIter = atoi(individualCommands[1]);

    /*
        Loop interations cannot be less than 1 
        or greater than maximum value of 4-byte integer variable
    */
    if(loopIter < 1 || loopIter > 0x7FFFFFFF){
        #if debug
            printf("Loop iterations either 0 or overflowing int buffer\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        free(individualCommands);
        return 1;
    }
    
    /*
        Copying the loop iterations into a string
        Needed to offset the command in main()
    */
    strcpy(loopIterStr, individualCommands[1]);
    free(individualCommands);
    return 0;
}

/**
 * @brief A function to check if the input string
 *        is completely made of whitespace characters * 
 * @param prompt String to be checked
 * @return int 0 if not a whitespace command, 1 otherwise
 */
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
    /*
        The executable doesn't accept any inputs
    */
    if(argc > 1){
        #if debug
            printf("No arguments accepted. Exiting.\n");
        #endif
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return 1;
    }
    
    /*
        Setting the default user input command size as 20
        getline() function can change this dynammically  
    */
    size_t promptSize = 20;
    
    while(1){
        char * prompt = (char *)malloc(sizeof(char) * promptSize);

        /*
            If whitespace characters, do nothing and get user input again
        */
        do{
            printf("smash> ");
            /*
                Since no \n in the previous statement, fflush() is used to flush the buffer
            */
            fflush(stdout);
            int charactersRead = getline(&prompt, &promptSize, stdin);
            if(charactersRead == -1){
                #if debug
                    printf("Failed to read command.\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(prompt);
                continue;
            }
        }while(whiteSpaceCommand(prompt));

        #if debug
            printf("Input Command: %s\n",prompt);
        #endif

        /*
            Making a copy of input command since
            strtok inside stringSplitter inside redirectErrorHandler & pipeErrorHandler is intrusive
            i.e., changes the contents of the string
        */
        char * promptDuplicate = strdup(prompt);
        /*
            Checking if the input command has a > or | to detect redirection or pipes
            Might be a bug in corner cases, haven't checked yet
        */
        char * ifRedirect = strchr(prompt,'>');
        char * ifPipe = strchr(prompt,'|');

        /*
            If both redirection and pipes are present,
            redirection shouldn't happen before pipe
            since redirection doesn't give any input to pipe
        */
        if(ifRedirect && ifPipe){
            if(strlen(ifRedirect) > strlen(ifPipe)){
                #if debug
                    printf("Cannot redirect twice\n");
                #endif
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(promptDuplicate);
                free(prompt);
                continue;
            }
        }
        
        /*
            Checking other errors that might occur with redirection or pipes
            If errors exist, continue (for the while loop) and get new input
        */
        if(ifRedirect)
            if(redirectErrorHandler(promptDuplicate)){
                free(promptDuplicate);
                free(prompt);
                continue;
            }
        if(ifPipe)
            if(pipeErrorHandler(promptDuplicate)){
                free(promptDuplicate);
                free(prompt);
                continue;
            }
        free(promptDuplicate);

        /*
            Splitting input command into multiple commands based on ;
        */
        char **multipleCommands = (char **)malloc(sizeof(char *) * (strlen(prompt) + 1));
        int numberOfCommands = stringSplitter(multipleCommands, prompt, ";");
        
        for(int i = 0; i < numberOfCommands; i++){
            char **individualCommands = (char **)malloc(sizeof(char *) * (strlen(multipleCommands[i]) + 1));
            /*
                Assume there are no loops present in the input
            */
            int ifLoop = 0;

            /*
                Checking if the command is a looping command
                Could have used char * ifLoop = strstr(multipleCommands[i], "loop");
                But if any file name or other executables named loops exists,
                then there will be a wrong detection
            */
            if( multipleCommands[i][0] == 'l' &&
                multipleCommands[i][1] == 'o' &&
                multipleCommands[i][2] == 'o' &&
                multipleCommands[i][3] == 'p'
              )
                ifLoop = 1;

            /*
                If the command has no loops,
                the command still has to run once
            */
            if(!ifLoop)
                loopIter = 1;

            if(ifLoop){
                /*
                    Limiting the looping iterations to the max value of 4-byte int
                    1 char extra for NULL
                */
                char loopIterStr[11] = "\0";
                char * loopCommand = strdup(multipleCommands[i]);
                /*
                    Checking for errors in the looping command implementation
                */
                if(loopErrorHandler(loopCommand, loopIterStr)){
                    free(loopCommand);
                    continue;
                }
                free(loopCommand);
                /*
                    Removing "loop # " out of the command by moving the pointer
                    strlen("loop") + 2x Whitespace + strlen(loopIterStr)
                    loopIterStr is the number of loop iterations set by loopErrorHandler()
                */
                multipleCommands[i] += (6 + strlen(loopIterStr));
            }
            
            char * loopCommand = strdup(multipleCommands[i]);
            for(int numOfLoops = 0; numOfLoops < loopIter; numOfLoops++){

                /*
                    Checking if the command inside loop is whitespace
                    Might be redundant (since already checked in loopErrorHandler())
                    But shoudn't affect the implementation 
                */
                if(whiteSpaceCommand(multipleCommands[i]))
                    continue;

                ifRedirect = strchr(multipleCommands[i], '>');
                ifPipe = strchr(multipleCommands[i], '|');
                if(actionHandler(multipleCommands[i], ifPipe, ifRedirect) < 0){
                    free(loopCommand);
                    break;
                }
                
                /*
                    Copying the same command back since we need to run in a loop
                    Also, multipleCommands[i] would have been modified by stringSplitter()
                */
                strcpy(multipleCommands[i], loopCommand);
            }
            free(loopCommand);
            free(individualCommands);
        }
        free(multipleCommands);
        free(prompt);
    }
    return 0;
}
