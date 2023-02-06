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
# include "constants/constants.h"

# include <stdlib.h>
# include <errno.h>

# define STRINGSIZE 100
/** LINESIZE has to support 512 whitespace/character pairs */
# define DELIMITER  " \t\n"

int main(void){
	/**
	 * \brief Main calling function that exclusively calls other functions *required to meet specifications*
	 *
	 * @return Exits to EXIT_FAILURE if utility functions such as print fails, otherwise exits to EXIT_SUCCESS when appropriate signal is sent
	 */

	pid_t pid;
	int status;

	// Variables needed for prompts declared outside the infinite loop to not create memory leaks
	char line[LINESIZE];
	size_t line_size = LINESIZE-1;
	
	// Runtime debug testing to make sure functions act according to how I want
	if(test_input() == EXIT_FAILURE){
		printf("Input functions fail runtime tests\n");
		exit(EXIT_FAILURE);
	};

	for(;;){

		// Specification requirement to identify child processes potentially exiting between loops
		while((pid = waitpid(0, &status, WNOHANG)) > 0) /** Waitpid set to 0 to make parent wait for child sharing the process group id */ {
			if(spec_check_for_child_background_processes(status, pid) != EXIT_SUCCESS){
				exit(EXIT_FAILURE);
			}
		}

		if(pid == 0){
			printf("Children exist but have not yet completed");
		} else if(pid < 0){
			perror("Error looking for children");
		};

		assert(pid < 1);

		// TODO: turn on a signal handler so that any signal prints a newline and for loop continues
		// Print prompt TODO: change stdin to file stream variable when implemented
		if(spec_get_line(line, line_size, stdin) == EXIT_SUCCESS){
			// Only in the singular case where we can verify input has succeeded are we heading out to any other function
			
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
