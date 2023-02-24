/**\brief Need this to compile with kill() and -std=c99 */
# define _POSIX_SOURCE
# define _GNU_SOURCE
# define _POSIC_C_SOURCE >= 200112L
# include "constants/constants.h"
# include "input/input.h"
# include "executes/executes.h"
# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>

# include <assert.h>
# include <stdint.h>

# include <stdbool.h>

# include <stdlib.h>
# include <errno.h>
# include "../signal-project/signal-project.h"

int main(void){
	/**
	 * \brief Main calling function that exclusively calls other functions *required to meet specifications*
	 *
	 * @return Exits to EXIT_FAILURE if utility functions such as print fails, otherwise exits to EXIT_SUCCESS when appropriate signal is sent
	 */

	spec_signal_handler();
	ParentStruct parent = {

		.last_foreground = {0},
		.last_background = {0},

	};
	for(;;){
		fflush(stdout);
		fflush(stderr);
		fflush(stdin);
		clearerr(stdin);
		clearerr(stdout);
		clearerr(stderr);
		ProgArgs current = {.command = {""}, .input = "", .output = "", .background = false};

		if(spec_check_for_child_background_processes(&parent) == EXIT_FAILURE){
			exit(EXIT_FAILURE);
		}
		if(strcmp(current.command[0], "") != 0){
			strcpy(current.command[0], "");
		}
		strcpy(current.input, "");
		strcpy(current.output, "");
		current.background = false;

		if(spec_execute(&current, stdin, &parent) == EXIT_FAILURE){
			perror("");
			
		};		
	}


	exit(EXIT_SUCCESS);
}
