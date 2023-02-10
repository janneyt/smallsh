# define _POSIX_SOURCE
# define _GNU_SOURCE
# define _POSIC_C_SOURCE >= 200112L

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif
# include "../expansion/expansion.h"
# include "../input/input.h"

int spec_parsing(char string[LINESIZE], struct ProgArgs *current){
	/**
	 * \brief Takes a *non-tokenized* string, tokenizes it, validates it, and sets internal arguments for execution of parent and child processes
	 *
	 * @param The character array with the commandline arguments
	 *
	 * @return EXIT_SUCCESS if every step completes, else EXIT_FAILURE for any step failing
	 * */
	char* storage[LINESIZE];
	int storagelength = 0;
	
	spec_word_splitting(storage, string);
	int index = 0;

	// Find the length of the char**
	while(index < LINESIZE){

		// Blank commands should simple returned with no commands set
		if(storage[index] == 0x0 && index == 0){
			return EXIT_SUCCESS;
		}
		if(storage[index] == 0x0){
			storagelength = index;
			index = LINESIZE;
		}


		index++;
	}

	// Now begin sorting and rejecting bad command line options
	index = 0;
	while(index < storagelength){

		// Lone & gets rejected
		if(index == 0 && strcmp(storage[index], "&") == 0){
			return EXIT_FAILURE;
		}

		// < should not be followed by > # or &
		if(strcmp(storage[index], "<") == 0 && index != storagelength - 1){

			// Rejection criteria
			if(strcmp(storage[index+1], ">") == 0 || strcmp(storage[index+1],"<") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "#") == 0 || strcmp(storage[index+1], "&") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "") == 0){
				return EXIT_FAILURE;
			} else if(storage[index+1][0] == '#' || storage[index+1][0] == '&'){
				return EXIT_FAILURE; 
			}

			strcpy(current->input, storage[index + 1]);
			
			index++;
		}

		// > should not be followed by < # or &
		if(strcmp(storage[index], ">") == 0 && index != storagelength - 1){

			// Rejection criteria
			if(strcmp(storage[index+1], ">") == 0 || strcmp(storage[index+1],"<") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "#") == 0 || strcmp(storage[index+1], "&") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "") == 0){
				return EXIT_FAILURE;
			} else if(storage[index+1][0] == '#' || storage[index+1][0] == '&'){
				return EXIT_FAILURE; 
			}

			strcpy(current->input, storage[index + 1]);
			
			index++;
		}
	
		index++;
	}


	return EXIT_SUCCESS;
}
