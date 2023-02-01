/**\brief Need this to compile with kill() and -std=c99 */
# define _POSIX_SOURCE
# define  _GNU_SOURCE

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

# define STRINGSIZE 100
# define LINESIZE   1000

/* Error Functions */

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

/* Utility functions */

int util_int_to_string(int num, char* str, int size){
	
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
	memset(str, 0, sizeof(*str));
	if(sprintf(temp, "%d", num) < 0){
		perror("Could not convert integer to string");
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(atoi(temp) == num);
	if(strlen(str) <= strlen(temp)){
		perror("Input string was too small to hold integer to string conversion value");
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(strlen(str) > strlen(temp));
	if(strcat(str, temp) == NULL){
		perror("Could not convert integer to string");
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(atoi(str) == num);
	return EXIT_SUCCESS;
}

int spec_check_for_child_background_processes(int status, pid_t pid){
	/**
	 * \brief Checks if a pid in the same process group id has completed either normally or in various abnormal ways.
	 * 
	 * @param status is the status of the pid in question
	 * @param pid is the process id of the process in question
	 *
	 * *Writes to stderr*
	 *
	 * `check_for_child_background_processes` is required to meet a specification requirement
	 *
	 * @return EXIT_FAILURE if printing to stderr didn't work, EXIT_SUCCESS if printing to stderr works.
	 */
	if (WIFEXITED(status)){
		if(fprintf(stderr, "Child process %jd done. Exit status %d.\n", (intmax_t) pid, WEXITSTATUS(status)) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};
	}
	else if (WIFSIGNALED(status)){
		assert(!WIFEXITED(status));
		if(fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) pid, WTERMSIG(status)) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};

		
	}
	else if (WIFSTOPPED(status)) {
		if(kill(pid, SIGCONT) != 0){
			assert(!WIFEXITED(status) && !WIFSIGNALED(status));
			perror("Kill didn't send a signal to continue, exiting");
			exit(EXIT_FAILURE);
		};
		// kill() never returns above 0, so initializing it to 1 indicates kill has not run
		int result = 1;
		if((result = kill(pid, SIGCONT)) != 0){
			perror("Kill didn't work");
			errno = 0;
			return EXIT_FAILURE;
		}
		if(fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid) < 0){
			perror("Could not print");
			assert(result == 0);
		}
		if(fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid) < 0){
			perror("Could not print error");
			errno = 0;
			return EXIT_FAILURE;
		};

	}
	assert(!WIFSTOPPED(status) && !WIFEXITED(status) && !WIFSIGNALED(status));
	return EXIT_SUCCESS;
}

int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream){
	/**
	 * \brief Gets the input and returns it. Has it's own signal handling.
	 * 
	 * @param input[] a character array 
	 * @param input_size is the size of the input *array*
	 * @param stream is a FILE* file pointer and allows for redirection.
	 *
	 * *Be careful to not overrun input*
	 *
	 * *This is a specification requirement function*
	 * @return EXIT_FAILURE if any part of the function errors, EXIT_SUCCESS if there's a valid input (and the full input) 
	 * in the passed input parameter
	 *
	 * */

	// input_length currently is zero, as nothing is entered. This would be the same as entering an empty string, so be careful
	printf("$");
	ssize_t input_length = 0;
	if(( input_length = getline(&input, &input_size, stream)) < 0){
		perror("Cannot fetch line from input");
		clearerr(stream);
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(input_length >= 0);
	return EXIT_SUCCESS;
}

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
	size_t n = 0;

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
		if(spec_get_line(line, n, stdin) == EXIT_SUCCESS){
			// Only in the singular case where we can verify input has succeeded are we heading out to any other function
			
		};
		clearerr(stdin); //TODO: change stdin to the file stream variable when implemented
		// TODO: turn off custom signal handler
		// Nothing goes below this line. Without input, there's no point in continuing processing. Instead, start the infinite loop again.
	}


	exit(EXIT_SUCCESS);
}
