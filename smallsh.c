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
# ifndef  handle_signal_exit
# include "utilities/utilities.h"
# include "error/error.h"
# include "input/input.h"
# include "expansion/expansion.h"
# include "parsing/parsing.h"

# include "execute/execute.h"
# endif
# ifndef  LINESIZE
# include "constants/constants.h"
# endif
# include <stdbool.h>

# include <stdlib.h>
# include <errno.h>

# include "builtins/builtins.h"
# include "execute/execute.h"


int main(void){
	/**
	 * \brief Main calling function that exclusively calls other functions *required to meet specifications*
	 *
	 * @return Exits to EXIT_FAILURE if utility functions such as print fails, otherwise exits to EXIT_SUCCESS when appropriate signal is sent
	 */

	ProgArgs current = {.command = {""}, .input = "", .output = ""};



	// Write a single test function using tester after it is debugged!
	printf("\nRoutine tests pass!\n");


	for(;;){

		if(spec_check_for_child_background_processes() == EXIT_FAILURE){
			perror("Had trouble waiting for child process");
			exit(EXIT_FAILURE);
		}

		if(spec_execute(&current, stdin) == EXIT_FAILURE){
			perror("\n**Error executing commands**\n\n");
		};		
	}


	exit(EXIT_SUCCESS);
}
