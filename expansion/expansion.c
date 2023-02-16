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
# ifndef LINESIZE
# include "../constants/constants.h"
# endif

# include <stdlib.h>
# include <errno.h>

int spec_expansion(char arg[LINESIZE], char substring[3], int control_code){
	/**
	 * \brief Expands four variables based on the specifications
	 *
	 * @param string is the string that needs to be scanned for expansion variables
	 *
	 * @return EXIT_SUCCESS for a successful scan (no expansion variables left) and and replace, EXIT_FAILURE for an unsuccessful scan or replace
	 * */

	// Maximum pid size for 64 bit systems is 9 digits long
	char str_pid[LINESIZE];

	// For some reason, getenv("HOME") is overwriting pointers so I'm saving the value before I declare anything
	// Substring == $$, to be replaced with the pid		
	if(control_code == 1){
		util_int_to_string(getpid(), str_pid, LINESIZE);

	} else if(control_code == 2){
		// Substring == ~/, replace with the HOME environment variable
		util_env_var_to_fixed_array("HOME", str_pid); 

	};
	char *discovery = strstr(arg, substring);
	char* temporary = "";

	int length = strlen(arg);

	if(length < 1 || length > LINESIZE){
		fprintf(stderr, "The length of the passed string is not in the range of 1 and %d inclusive", LINESIZE);
		exit(EXIT_FAILURE);
	}

	// No need to process if current substring hasn't been found
	if(discovery == NULL){
		if(strcmp(substring, "$$") == 0){
			return spec_expansion(arg, "~/", 2);
		} else if(strcmp(substring, "~/") == 0){
			return EXIT_SUCCESS;
		}
			
	};
	



	// This exhaustively searches for the substrings
	while(discovery != NULL){


		// -2 because length-1 indexes the final character and thus the substring with size 2 needs an offset of 2
		if(&discovery[0] != &(arg[length - 2])){
			temporary = strdup(discovery);
			temporary += 2;
		} 
		if( &arg[0] == &discovery[0] && control_code == 2){
			*discovery = '\0';
			discovery++;
			*discovery = '\0';
			strcat(arg, str_pid);
			strcat(arg, temporary);
			return EXIT_SUCCESS;	
		}

		// Mainstream, non-edge case processing where the discovery is not at the front of the string
		else if( &arg[0] != &discovery[0] && control_code != 2){
			*discovery = '\0';
			discovery++;
			*discovery = '\0';

			strcat(arg, str_pid);
			strcat(arg, temporary);
		} else if (control_code != 2){
			*discovery = '\0';
			discovery++;
			*discovery = '\0';

			strcat(arg, str_pid);
			strcat(arg, temporary);
		} else if (control_code == 2){
			arg[length] = '\0';
			return EXIT_SUCCESS;
		}
		discovery = strstr(arg, substring);
		
	};

	strcat(arg, "");
	
	if(control_code == 1){
		return spec_expansion(arg, "~/", 2);
	}
	return EXIT_SUCCESS;

}
