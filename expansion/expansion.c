/**\brief Need this to compile with kill() and -std=c99 */
# define _POSIX_SOURCE
# define _GNU_SOURCE
# define _POSIC_C_SOURCE >= 200112L

# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include "../utilities/utilities.h"
# include "../error/error.h"
# include "../tester/tester.h"
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif

# include <stdlib.h>
# include <errno.h>

# define STRINGSIZE 100
/** LINESIZE has to support 512 whitespace/character pairs */
# define DELIMITER  " \t\n"

int spec_expansion(char arg[LINESIZE], char substring[3], int control_code){
	/**
	 * \brief Expands four variables based on the specifications
	 *
	 * @param string is the string that needs to be scanned for expansion variables
	 *
	 * @return EXIT_SUCCESS for a successful scan (no expansion variables left) and and replace, EXIT_FAILURE for an unsuccessful scan or replace
	 * */
	int length = strlen(arg);
	char str_pid[10];

	// For some reason, getenv("HOME") is overwriting pointers so I'm saving the value before I declare anything
	// Substring == $$, to be replaced with the pid		
	if(control_code == 1){
		util_int_to_string(getpid(), str_pid, 10);
	} else if(control_code == 2){
		// Substring == ~/, replace with the HOME environment variable
		strcpy(str_pid, getenv("HOME")); 

	};
	char *discovery = strstr(arg, substring);
	char* temporary = "";
	char* initial = "";

	// No need to process if current substring hasn't been found
	if(discovery == NULL){
		if(strcmp(substring, "$$") == 0){
			return spec_expansion(arg, "~/", 2);
		} else if(strcmp(substring, "~/") == 0){
			return EXIT_SUCCESS;
		}
			
	};
	
	// If the first substring doesn't *start* the whole string, set the initial pointer to be discovery - 1
	if(&arg[0] != &discovery[0]){
		initial = discovery - 1;
	} 

	// Although LINESIZE is a global constant in the constants file, this enforces the size
	if(length < 1 || length > LINESIZE){
		fprintf(stderr, "The length is not in the range 1 and %d includsive", LINESIZE);
		return EXIT_FAILURE;
	};

	// This exhaustively searches for the substrings
	while(discovery != NULL){


		// -2 because length-1 indexes the final character and thus the substring with size 2 needs an offset of 2
		if(&discovery[0] != &(arg[length - 2])){
			temporary = strdup(discovery);
			temporary += 2;
		} else {
			strcpy(temporary, "");
		};

		// Mainstream, non-edge case processing where the discovery is not at the front of the string
		if(&initial[0] != &discovery[0]){	
			*discovery = '\0';
			strcat(initial, str_pid);
			strcat(initial, temporary);
		} else {
			strcat(str_pid, temporary);
			arg = str_pid;
		}
		discovery = strstr(arg, substring);
		
	};
	
	if(control_code == 1){
		return spec_expansion(arg, "~/", 2);
	}
	free(temporary);
	return EXIT_SUCCESS;

}
