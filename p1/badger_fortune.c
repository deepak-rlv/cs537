#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

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

	int fortuneNumber = 0;

	bool optF = 0, optN = 0, optB = 0, optO = 0;
	char fortuneFileName[255];
	char batchFileName[255];

	FILE *fortuneFile = NULL;
	FILE *outputFile = NULL;
	FILE *batchFile = NULL;

	/* Looping until the return value is -1 i.e., no more arguments to parse */
	while((argument = getopt(argc, argv, "+f:n:b:o:")) != -1){
		switch(argument){
			case 'f':
				optF = 1;

				strcpy(fortuneFileName,optarg);

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

				fortuneNumber = atoi(optarg);

				#if DEBUG
					printf("Fortune Number: %d\n", fortuneNumber);
				#endif

				break;

			case 'b':
				optB = 1;

				strcpy(batchFileName,optarg);

				if(optN){
					printf("ERROR: You can't use batch mode when specifying a fortune number using -n\n");
					return 1;
				}
				
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
		}
	}

	if(!optF){
		printf("ERROR: No fortune file was provided\n");
		return 1;
	}

	char totalFortunes_string[10];
	char maximumFortuneLength_string[10];

	fortuneFile = fopen(fortuneFileName,"r");
	if(fortuneFile == NULL){
		printf("ERROR: Can't open fortune file\n");
		return 1;
	}

	if(fgets(totalFortunes_string,10,fortuneFile) == NULL){		//change to get line
		printf("ERROR: Fortune File Empty\n");
		return 1;
	}
	if(fgets(maximumFortuneLength_string,10,fortuneFile));		//change to get line

	const int maximumFortuneLength = atoi(maximumFortuneLength_string);
	const int totalFortunes = atoi(totalFortunes_string);

	if(optN){
		if(fortuneNumber <= 0 || fortuneNumber > totalFortunes){
			printf("ERROR: Invalid Fortune Number\n");
			return 1;
		}
	}

	int fortunesHit = 0; //denotes the head of the fortune and not tail

	char readCharacter[2];
	readCharacter[1] = '\0';

	char *fortunesList = malloc(totalFortunes * maximumFortuneLength * sizeof(char));

    for(int i = 0, j = 0; i < totalFortunes; ){
        while(1){
            readCharacter[0] = fgetc(fortuneFile);
            if(readCharacter[0] == '%'){
                readCharacter[0] = fgetc(fortuneFile);
                if(readCharacter[0] == '\n'){
                    fortunesHit++;
                    break;
                }
                else{
                    readCharacter[0] = fgetc(fortuneFile);
                    fortunesList[i * maximumFortuneLength + j] = readCharacter[0];
                    j++;
                }
            }
            else{
                fortunesList[i * maximumFortuneLength + j] = readCharacter[0];
                j++;
            }
        }
        if(fortunesHit > 1){
            j = 0;
            i++;
        }
    }

	if(optN){
		printf("%s", &fortunesList[(fortuneNumber - 1) * maximumFortuneLength]);
	}
	else if(optB){
		batchFile = fopen(batchFileName, "r");
		if(batchFile == NULL){
			printf("ERROR: Can't open batch file\n");
			return 1;
		}

		fortuneNumber = 0;

		if(fscanf(batchFile, "%d", &fortuneNumber) == EOF){
			printf("ERROR: Batch File Empty\n");
			return 1;
		}

        do{
			if(fortuneNumber <= 0 || fortuneNumber > totalFortunes)
				printf("ERROR: Invalid Fortune Number\n\n");
			else
        		printf("%s\n\n", &fortunesList[(fortuneNumber - 1) * maximumFortuneLength]);
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
