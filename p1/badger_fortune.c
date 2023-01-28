/*
	Written by Deepak Charan Logavaseekaran 
	UW Madison - CS537 - SP23 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

/* Set to 1 enables debugging outputs */
#ifndef DEBUG
#define DEBUG 0
#endif

int main(int argc, char *argv[]){
	if(argc < 5 || argc > 7){
		printf("USAGE: \n\tbadger-fortune -f <file> -n <number> (optionally: -o <output file>) "
		"\n\t\t OR \n\tbadger-fortune -f <file> -b <batch file> (optionally: -o <output file>)\n");
		return 1;
	}

	#if DEBUG
		printf("***** DEBUG MODE *****\n");
	#endif

	/* Disabling the error printed out by the getopt function */
	opterr = 0;

	/* Variable to hold the command line argument (if present) else hold the error returned by the function */
	int argument;
	/* Variable to hold the fortune number given by either -n or -b */
	int fortuneNumber = 0;
	/* Variable to hold the total number of fortunes in a fortune.txt file (given by the 1st line) */
	int totalFortunes = 0;
	/* Variable to hold the total characters in the largest fortune in a fortune.txt file (given by the 2nd line) */
	int maximumFortuneLength = 0;
	/* Variable to hold the number of fortunes hit while seeking through the file (tracks the head & not the tail)*/
	int fortunesHit = 0;
	/* Variable to hold the character read while seeking through the file */
	char readCharacter;

	bool optF = 0, optN = 0, optB = 0, optO = 0;
	char fortuneFileName[255];
	char batchFileName[255];

	FILE *fortuneFile = NULL;
	FILE *outputFile = NULL;
	FILE *batchFile = NULL;

	/*
		Looping until the return value is -1 i.e., no more arguments to parse	
		'+' specifies that the getopt funtions stops after the first non-option hit
		':' specifies that a value is mandatory for that particular option
		Ref: https://www.gnu.org/software/libc/manual/html_node/Using-Getopt.html
	*/
	while((argument = getopt(argc, argv, "+f:n:b:o:")) != -1){
		switch(argument){
			case 'f':
				optF = 1;
				
				/* 'optarg' holds the argument for the corresponding option, the fortune filename in this case */
				strcpy(fortuneFileName, optarg);

				#if DEBUG
					printf("Fortune File's name: %s\n",optarg);
				#endif

				break;
			case 'n':
				optN = 1;
				if(optB){
					printf("ERROR: You can't specify a specific fortune number in conjunction with batch mode\n");
					return 1;
				}

				/* 'optarg' holds the argument for the corresponding option, the fortune number in this case */
				fortuneNumber = atoi(optarg);

				#if DEBUG
					printf("Fortune Number: %d\n", fortuneNumber);
				#endif

				break;
			case 'b':
				optB = 1;
				if(optN){
					printf("ERROR: You can't use batch mode when specifying a fortune number using -n\n");
					return 1;
				}

				/* 'optarg' holds the argument for the corresponding option, the batch filename in this case */
				strcpy(batchFileName,optarg);
				
				#if DEBUG
					printf("Batch File's name: %s\n",optarg);
				#endif

				break;
			case 'o':
				optO = 1;

				outputFile = fopen(optarg,"w");

				#if DEBUG
					printf("Output File's name: %s\n",optarg);
				#endif

				break;
			case '?':
				printf("ERROR: Invalid Flag Types\n");
				return 1;
			default:
				printf("ERROR: Invalid Flag Types\n");
				return 1;
		}
	}

	/* Checking is option F for mentioned in the command line */
	if(!optF){
		printf("ERROR: No fortune file was provided\n");
		return 1;
	}
	fortuneFile = fopen(fortuneFileName,"r");
	if(fortuneFile == NULL){
		printf("ERROR: Can't open fortune file\n");
		return 1;
	}

	/*
		Fortune file is empty if reading the first line returns EOF
		If not, the data is read as integer to store the total number of fortunes in the file
	*/
	if(fscanf(fortuneFile, "%d\n", &totalFortunes) == EOF){
		printf("ERROR: Fortune File Empty\n");
		return 1;
	}

	/*
		Print error and exit if fscanf reads more than 1 element
		If not, the data is read as integer denoting the string length of the largest fortune
	*/
	if(fscanf(fortuneFile, "%d\n", &maximumFortuneLength) != 1){
		printf("ERROR: Maximum Fortune Size not as per format\n");
		return 1;
	}

	/* Print error message and return if -n has argument <= 0 or more than the total available fortunes */
	if(optN){
		if(fortuneNumber <= 0 || fortuneNumber > totalFortunes){
			printf("ERROR: Invalid Fortune Number\n");
			return 1;
		}
	}

	/* Holds the list of all fortunes available in the fortune file */
	char *fortunesList = malloc(totalFortunes * maximumFortuneLength * sizeof(char));

	/*
		Loops through and stores the entire fortune.txt file in fortunesList
		Checks for consecutive '%' and '\n', denoting the start of a fortune, and increments fortunesHit counter
		
		[i * maximumFortuneLength + j] is basically [i][j] with each row having maximumFortuneLength character
	*/
    for(int i = 0, j = 0; i < totalFortunes; ){
        while(1){
            readCharacter = fgetc(fortuneFile);
            if(readCharacter == '%'){
                readCharacter = fgetc(fortuneFile);
                if(readCharacter == '\n'){
                    fortunesHit++;
                    break;
                }
                else{
                    readCharacter = fgetc(fortuneFile);
                    fortunesList[i * maximumFortuneLength + j] = readCharacter;
                    j++;
                }
            }
            else{
                fortunesList[i * maximumFortuneLength + j] = readCharacter;
                j++;
            }
        }
        if(fortunesHit > 1){
            j = 0;
            i++;
        }
    }

	/* Print only one fortune as per input if -n is specified */
	if(optN){
		printf("%s", &fortunesList[(fortuneNumber - 1) * maximumFortuneLength]);	// Using fortuneNumber-1 since index start from 0
	}
	else if(optB){
		batchFile = fopen(batchFileName, "r");
		if(batchFile == NULL){
			printf("ERROR: Can't open batch file\n");
			return 1;
		}

		fortuneNumber = 0;

		/* Batch file empty if the first read returns EOF */
		if(fscanf(batchFile, "%d", &fortuneNumber) == EOF){
			printf("ERROR: Batch File Empty\n");
			return 1;
		}

		/* 
			Using do..while since the first number will already be processed
			if the previous if condition fails
		*/
        do{
			/* Print error message and continue if batch file has argument <= 0 or more than the total available fortunes */
			if(fortuneNumber <= 0 || fortuneNumber > totalFortunes)
				printf("ERROR: Invalid Fortune Number\n\n");
			else
        		printf("%s\n\n", &fortunesList[(fortuneNumber - 1) * maximumFortuneLength]);	// Using fortuneNumber-1 since index start from 0
		} while(fscanf(batchFile, "%d", &fortuneNumber) != EOF);
	}


	#if DEBUG
		printf("%d\n",totalFortunes);
		printf("%d\n",maximumFortuneLength);
	#endif

	free(fortunesList);
	fclose(fortuneFile);
	if(optB)
		fclose(batchFile);
	if(optO)
		fclose(outputFile);
	
	return 0;
}
