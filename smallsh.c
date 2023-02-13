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
# include "utilities/utilities.h"
# include "error/error.h"
# include "input/input.h"
# include "tester/tester.h"
# include "expansion/expansion.h"
# include "parsing/parsing.h"
# ifndef  LINESIZE
# include "constants/constants.h"
# endif
# include <stdbool.h>

# include <stdlib.h>
# include <errno.h>

# include "../builtins/builtins.h"


int main(void){
	/**
	 * \brief Main calling function that exclusively calls other functions *required to meet specifications*
	 *
	 * @return Exits to EXIT_FAILURE if utility functions such as print fails, otherwise exits to EXIT_SUCCESS when appropriate signal is sent
	 */

	pid_t pid;
	int status;
	char child_action[LINESIZE];

	char holder[LINESIZE];

	// Variables needed for prompts declared outside the infinite loop to not create memory leaks
	char line[LINESIZE];
	size_t line_size = LINESIZE-1;
	
	struct ProgArgs current;
	strcpy(current.command, "");
	strcpy(current.input, "");
	strcpy(current.output, "");
	current.background = false;

	// Runtime debug testing to make sure functions act according to how I want
	if(test_input() == EXIT_FAILURE){
		printf("Input functions fail runtime tests\n");
		exit(EXIT_FAILURE);
	};
	if(test_expansion() == EXIT_FAILURE){
		exit(EXIT_FAILURE);
	};
	
	if(test_parsing(&current) == EXIT_FAILURE){
		exit(EXIT_FAILURE);
	}
	printf("\nRoutine tests pass!\n");

	if(spec_check_for_child_background_processes() == EXIT_FAILURE){
		perror("Had trouble waiting for child process");
		exit(EXIT_FAILURE);
	}
	for(;;){

		if(spec_execute(&current) == EXIT_FAILURE){
			perror("");
		};		
	}


	exit(EXIT_SUCCESS);
}
