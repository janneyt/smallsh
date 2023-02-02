# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include <stdlib.h>
# include <errno.h>

int err_child_error(char child_action[], char action_taken[]){
	/**
	 * \brief Produces an error because a child process has completed in a systematic way.
	 *
	 * @param child_action is a string of arbitrary size that represents *what* caused completion
	 * @param action_taken is a string of arbtrary size that represents *what was the result* such as continuing or exiting.
	 *
	 * *Writes to stderr*
	 *
	 * @return EXIT_SUCCESS if process successfully writes to stderr, EXIT_FAILURE if there was an error while writing
	 */
	if(fprintf(stderr, "Child process %s. %s.", child_action, action_taken) < 0){
		perror("Could not print error");
		errno = 0;
		return EXIT_FAILURE;
	};
	return (EXIT_SUCCESS);
}

