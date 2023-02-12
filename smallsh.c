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

	for(;;){

		// Specification requirement to identify child processes potentially exiting between loops
		while((pid = waitpid(1, &status, WNOHANG)) > 0){
		
			child_action[0] = '\0';


			if(spec_check_for_child_background_processes(status, pid) != EXIT_SUCCESS){
				exit(EXIT_FAILURE);
			}
			if(WIFEXITED(status)){
				strcat(child_action, "Process ");
				util_int_to_string(pid, holder, 10);
				strcat(child_action, holder);
				strcat(child_action, " exited with status: ");
				util_int_to_string(pid, holder, 10);
				strcat(child_action, holder);
				printf("%s", child_action);
			}
		}

		printf("%ul\n", pid);
		
		if(pid == 0){
			printf("Children exist but have not yet completed");
		} else if(pid < 0){
			printf("%ul\n", pid);
			perror("");
		};

		assert(pid < 1);

		// TODO: turn on a signal handler so that any signal prints a newline and for loop continues
		// Print prompt TODO: change stdin to file stream variable when implemented
		if(spec_get_line(line, line_size, stdin) == EXIT_SUCCESS){
			// Only in the singular case where we can verify input has succeeded are we heading out to any other function
			if(spec_expansion(line, "$$", 1) == EXIT_SUCCESS){
				if(spec_parsing(line, &current) == EXIT_SUCCESS){
					printf("Successfully parsed\n");
				} else {
					perror("");
					printf("Could not parse line\n");
				}
			} else {
				printf("Could not expand variables\n");
			}
			
		} else {
			printf("error with spec_get_line");
			exit(EXIT_FAILURE);
		};
		clearerr(stdin); //TODO: change stdin to the file stream variable when implemented
		// TODO: turn off custom signal handler
		// Nothing goes below this line. Without input, there's no point in continuing processing. Instead, start the infinite loop again.
	}


	exit(EXIT_SUCCESS);
}
