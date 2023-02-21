# define _POSIX_SOURCE
# define _GNU_SOURCE
# define _POSIC_C_SOURCE >= 200112L

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <stdbool.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif
# include "../expansion/expansion.h"
# include "../input/input.h"
# include "../builtins/builtins.h"

int spec_parsing(char string[LINESIZE], ProgArgs *current){
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
	int command_index = 0;
        current->background = false;

	if(storage[0] == 0x0){
		fprintf(stderr, "Input is equal to null, which is invalid");
		return EXIT_FAILURE;
	}

	// Find the length of the char**
	while(index < LINESIZE){

		// Blank commands should simple returned with no commands set
		if(storage[index] == 0x0 && index == 0){
			return EXIT_SUCCESS;
		}

		if(storage[index] == 0x0){
			storagelength = index;
			index = LINESIZE;
		} else if(strcmp(storage[index], "#") == 0){
			storagelength = index;
			index = LINESIZE;

		}

		index++;
	}

	// Now begin sorting and rejecting bad command line options
	index = 0;


	while(index < storagelength){

		// #< or #file1 etc are still comments
		if(storage[index][0] == '#'){
			index = storagelength;
			continue;
		}

		// Lone & gets rejected
		if(index == 0 && strcmp(storage[index], "&") == 0){
			current->background = false;
			return EXIT_FAILURE;
		}

		// < or > at the end are automatically invalid
		else if(index == storagelength -1){
			if(strcmp(storage[index], "<") == 0) {
				return EXIT_FAILURE;
			} else if(strcmp(storage[index], ">") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index], "&") == 0){
				current->background = true;
				
			} else {
				strcat(storage[index], "");
				current->command[command_index] = storage[index];
				command_index++;

			}
		}
		// < should not be followed by > # or &
		else if(strcmp(storage[index], "<") == 0 && index != storagelength - 1){

			// Rejection criteria
			if(strcmp(storage[index+1], ">") == 0 || strcmp(storage[index+1],"<") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "#") == 0 || strcmp(storage[index+1], "&") == 0){
				return EXIT_FAILURE;
			} else if(strcmp(storage[index+1], "") == 0){
				return EXIT_FAILURE;
			} else if(storage[index+1][0] == '#' || storage[index+1][0] == '&'){
				return EXIT_FAILURE; 
			} else if(strcmp(storage[index], ">") == 0 && index == storagelength){
				return EXIT_FAILURE;
			} 

			// The valid options for redirection operation + 2 are & or # or the other redirection operator
			if(storage[index+2] && (strcmp(storage[index+2], "#") != 0 && strcmp(storage[index+2], "&") != 0 && strcmp(storage[index+2], ">") != 0)){
				return EXIT_FAILURE;
			}

			strcpy(current->input, storage[index+1]);
			
			index++;
		}

		// > should not be followed by < # or &
		else if(strcmp(storage[index], ">") == 0 && index != storagelength - 1){

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

			// The valid options for redirection operation + 2 are & or # or the other redirection operator
			if(storage[index+2] && (strcmp(storage[index+2], "#") != 0 && strcmp(storage[index+2], "&") != 0 && strcmp(storage[index+2], "<") != 0)){
				return EXIT_FAILURE;
			}

			strcpy(current->output, storage[index + 1]);

			index++;
		}

		// & must only be at the end, all other invalid
		else if(strcmp(storage[index], "&") == 0 && index == storagelength -1){
			current->background = true;
			return EXIT_SUCCESS;
		} else if (strcmp(storage[index], "&") == 0 && index != storagelength -1){
			return EXIT_FAILURE;
		}

		// Having weeded out everything else, we now have to assemble a command line with options by reconcatenating
		// THIS DOES NOT NEED TO BE A VALID PROGRAM, we will let the operating system handle that
		else {
			if(storage[index] == 0x0){
				return EXIT_FAILURE;
			}
			strcat(storage[index], "");
			current->command[command_index] = storage[index];
			command_index++;
	
		}

		index++;
	}

	current->command[command_index] = 0x0;

	return EXIT_SUCCESS;
}
