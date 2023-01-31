/**
 * # Imports #
 */
# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

/**
 * # Macros #
 */

# define 


/**
 * # Code
 */
# Code #

/**
 * ## Error handling functions
 */
int child_error(char child_action[], char action_taken[]){
	/**
	 * \brief Produces an error because a child process has completed in a systematic way.
	 *
	 * @param child_action is a string of arbitrary size that represents *what* caused completion
	 * @param action_taken is a string of arbtrary size that represents *what was the result* such as continuing or exiting.
	 *
	 * Writes to stderr.
	 *
	 * @return EXIT_SUCCESS if process successfully writes to stderr, EXIT_FAILURE if there was an error while writing
	 */
	fprintf(stderr, "Child process %s. %s.", child_action, action_taken);
	return (EXIT_SUCCESS);
}

/**
 * ## Utility functions 
 */

int int_to_string(int num, char* str, int size){
	
	/**
	 * \brief Converts an integer to its strings representation
	 *
	 * @param num is the integer that needs to be converted to a string
	 * @param str is the holder string that stores the integer representation
	 * @param size is the maximum length possible to store in str, including \0
	 * @return EXIT_SUCCESS if integer has been entirely converted to 
	 * string, EXIT_FAILURE if integer cannot be convered to string
	 *
	 */

	char temp[size];
	if(sprintf(temp, "%d", num) < 0){
		perror("Could not convert integer to string");
		return EXIT_FAILURE;
	};
	if(strlen(str) <= strlen(temp)){
		return EXIT_FAILURE;
	}
	if(strcat(str, temp) == NULL){
		perror("Could not convert integer to string");
		return EXIT_FAILURE;
	};
	return EXIT_SUCCESS;
}

/**
 * ## Functions needed to meet specification requirements
 */

int check_for_child_background_processes(int status, int pid){
	int  string_size = 100;
	char child_action[string_size];
	char action_taken[string_size];
	memset(child_action, 0, sizeof(child_action));
	memset(action_taken, 0, sizeof(action_taken));
	assert(child_action == 0x0);
	assert(action_taken == 0x0);

	if (WIFEXITED(status)){
		strcat(child_action,"done");
		strcat(action_taken, "Exit value ");
		if(int_to_string(WEXITSTATUS(status), action_taken, string_size) != EXIT_SUCCESS){
			 return EXIT_FAILURE;
		};

	}
	else if (WIFSIGNALED(status)){
		strcat(child_action, "done");
		strcat(action_taken, "Signaled ");
		if(int_to_string(WTERMSIG(status), action_taken, string_size) != EXIT_SUCCESS){
			return EXIT_FAILURE;
		};

	}
	else if (WIFSTOPPED(status)) {
		strcat(child_action, "stopped");		
		strcat(action_taken,"Continuing");
		if(kill(pid, SIGCONT) != 0){
			perror("Kill didn't work");
			return EXIT_FAILURE;
		}

	}
	return child_error(child_action, action_taken);
}

int main(){

	pid_t pid;
	int status;
	
	for(;;){

		// Specification requirement to identify child processes potentially exiting between loops
		while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			if(check_for_child_background_processes(status, pid) != EXIT_SUCCESS){
				exit(EXIT_FAILURE);
			}
		}

		// Print prompt
	}


	exit(EXIT_SUCCESS);
}
