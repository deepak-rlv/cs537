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

int actionHandler(char * prompt){
    char **args = malloc(sizeof(char) * (strlen(prompt) + 1));

    int i=0;
    while(1){
        args[i] = strsep(&prompt," ");
        if(args[i] == (char *) NULL)
            break;
        i++;
    }

    if(!strcmp(args[0],"exit")){
        exit(0);
    } else if(!strcmp(args[0],"cd")){
        if(args[1] == NULL || args[2] != NULL){
            printf("cd arguments error\n");
        }
        int chdirReturn = chdir(args[1]);
        if(chdirReturn < 0){
            printf("chdir error\n");
        }
    } else if(!strcmp(args[0],"pwd")){
        char currentDir[PWD_SIZE];
        int pwdReturn = getcwd(currentDir, PWD_SIZE);
        printf("%s\n",currentDir);
        return 0;
    } else {
        int forkReturn = fork();
        if(forkReturn < 0){
            #if debug
                printf("Fork failed\n");
            #endif
            return 1;
        } else if(forkReturn == 0){
            int execReturn = execvp(args[0],args);
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
                return 1;
            }
        }
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

        actionHandler(prompt);

    }

    exit(0);

    return 0;
}