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
	if(argc < 5){
		printf("USAGE: \n\tbadger-fortune -f <file> -n <number> (optionally: -o <output file>)	\
		\n\t\t OR \n\tbadger-fortune -f <file> -b <batch file> (optionally: -o <output file>)\n");
		return 1;
	}

	#if DEBUG
		printf("***** DEBUG MODE *****\n");
	#endif

	/* Disabling the error printed out by the getopt function */
	opterr = 0;

	/* Variable to hold the command line argument (if present) else hold the error returned by the function */
	int argument;

	bool /* optF = 0, */ optN = 0, optB = 0/* , optO = 0 */;

	/* Looping until the return value is -1 i.e., no more arguments to parse */
	while((argument = getopt(argc, argv, "f:n:b:o:")) != -1){
		switch(argument){
			case 'f':
				// optF = 1;
				FILE *fortuneFile;
				fortuneFile = fopen(optarg,"r");

				#if DEBUG
					printf("Fortune File's name: %s\n",optarg);
				#endif

				if(fortuneFile == NULL){
					printf("ERROR: Can't open fortune file\n");
					return 1;
				}
				break;

			case 'n':
				optN = 1;

				if(optB){
					printf("ERROR: You can't specify a specific fortune number in conjunction with batch mode\n");
					return 1;
				}

				int numberOfFortunes = atoi(optarg);

				#if DEBUG
					printf("Number of fortunes: %d\n", numberOfFortunes);
				#endif

				numberOfFortunes++;
				break;

			case 'b':
				optB = 1;

				if(optN){
					printf("ERROR: You can't use batch mode when specifying a fortune number using -n\n");
					return 1;
				}

				FILE *batchFile;
				batchFile = fopen(optarg,"r");
				#if DEBUG
					printf("Batch File's name: %s\n",optarg);
				#endif
				if(batchFile == NULL)
					printf("ERROR: Can't open batch file\n");
				break;

			case 'o':
				// optO = 1;

				// FILE *outputFile;
				// outputFile = fopen(optarg,"w");
				// #if DEBUG
				// 	printf("Output File's name: %s\n",optarg);
				// #endif
				// if(outputFile == NULL)
				// 	printf("ERROR: Can't open output file\n");
				break;

			case '?':
				printf("ERROR: Invalid Flag Types\n");
				break;
		}
	}
	
	return 0;
}
