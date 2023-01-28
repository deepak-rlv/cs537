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

	bool /* optF = 0, */ optN = 0, optB = 0, optO = 0;
	char fortuneFileName[255];
	char batchFileName[255];
	int numberOfFortunes = 0;

				FILE *fortuneFile = NULL;
				FILE *outputFile = NULL;
				FILE *batchFile = NULL;
	/* Looping until the return value is -1 i.e., no more arguments to parse */
	while((argument = getopt(argc, argv, "+f:n:b:o:")) != -1){
		switch(argument){
			case 'f':
				/* optF = 1; */

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

				numberOfFortunes = atoi(optarg);

				#if DEBUG
					printf("Number of fortunes: %d\n", numberOfFortunes);
				#endif

				numberOfFortunes++;
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

	char totalFortunes_string[10];
	char maximumFortuneLength_string[10];

	fortuneFile = fopen(fortuneFileName,"r");
	if(fortuneFile == NULL){
		printf("ERROR: Can't open fortune file\n");
		return 1;
	}

	if(fgets(totalFortunes_string,10,fortuneFile));		//change to get line
	if(fgets(maximumFortuneLength_string,10,fortuneFile));		//change to get line

	const int maximumFortuneLength = atoi(maximumFortuneLength_string);

	char readCharacter[2];
	readCharacter[1] = '\0';

	if(optN){
		for(int i = 0; i < numberOfFortunes; i++){
			char fortune[maximumFortuneLength];
			while(strlen(fortune) < maximumFortuneLength){
				readCharacter[0] = fgetc(fortuneFile);
				if(readCharacter[0] == '%'){
					readCharacter[0] = fgetc(fortuneFile);
					if(readCharacter[0] == '\n'){
						break;
					}
					else{
						readCharacter[0] = fgetc(fortuneFile);
						strcat(fortune,readCharacter);
					}
				}
				else{
						strcat(fortune,readCharacter);
				}
			}
			if(!optO){
				if(strlen(fortune)!=0)
					printf("%s\n\n",fortune);
			}
			else{
				//write to -o file
			}
		}
	}
	else if(optB){
		batchFile = fopen(batchFileName,"r");
		if(batchFile == NULL){
			printf("ERROR: Can't open batch file\n");
			return 1;
		}
		for(int i = 0; i < numberOfFortunes; i++){ // use getline in the for loop to iterate through all the fortunes
			char fortune[maximumFortuneLength];
			while(strlen(fortune) < maximumFortuneLength){
				readCharacter[0] = fgetc(fortuneFile);
				if(readCharacter[0] == '%'){
					readCharacter[0] = fgetc(fortuneFile);
					if(readCharacter[0] == '\n'){
						break;
					}
					else{
						readCharacter[0] = fgetc(fortuneFile);
						strcat(fortune,readCharacter);
					}
				}
				else{
						strcat(fortune,readCharacter);
				}
			}
			if(!optO){
				if(strlen(fortune)!=0)
					printf("%s\n\n",fortune);
			}
			else{
				//write to -o file
			}
		}
	}
	

	#if DEBUG
		const int totalFortunes = atoi(totalFortunes_string);
		printf("%d\n",totalFortunes);
		printf("%d\n",maximumFortuneLength);
	#endif

	fclose(fortuneFile);
	if(optO)
		fclose(outputFile);
	
	return 0;
}
