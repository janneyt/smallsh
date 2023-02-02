# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
# include "../constants/constants.h"

# include <stdlib.h>
# include <errno.h>

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
	ssize_t input_length = 0;

	// This program is constantly setting and resetting errno, so if errno has a value it hasn't been caught elsewhere
	if(errno != 0){
		errno = 0;
	};
	assert(errno == 0);

	// PS1 print
	printf("$");
	if(( input_length = getline(&input, &input_size, stream)) < 0){
		perror("Cannot fetch line from input");
		clearerr(stream);
		errno = 0;
		return EXIT_FAILURE;
	};
	assert(input_length >= 0);
	return EXIT_SUCCESS;
}

char** help_split_line(char** storage, char* line){
	/**
	 * \brief Helper function that tokenizes a line into an array of words
	 *
	 * @param line an array, with delimiters, of words needing to be tokenized
	 * @param storage is an array of points needed to hold the tokenized words
	 * @return an array of tokens
	 * */
	int 	bufsize = LINESIZE;
	int 	position = 0;
	char*	token;
	char*	delim = getenv("IFS");
	int	token_bufsize = 64;
	char**  array_of_tokens = storage;

	delim = (delim != 0x0) ? delim : DELIMITER;
	
	if(bufsize < 1){
		printf("A buffer of 1 or more is needed for tokenization");
		exit(EXIT_FAILURE);
	};

	// TODO: variable expansion needed here
	
	token = strtok(line, delim);
	while(token != NULL){
		array_of_tokens[position] = token;
		position++;
		if(position >= bufsize){
			bufsize += token_bufsize;
			if((array_of_tokens = realloc(array_of_tokens, bufsize * sizeof(char*))) == NULL){
				perror("Cannot reallocate memory for array of tokens");
				exit(EXIT_FAILURE);
			};
			
		}
		token = strtok(NULL, delim);
	};

	array_of_tokens[position] = NULL;
	return array_of_tokens;

} 

int spec_word_splitting(char* storage[LINESIZE], char input[LINESIZE]){
	/**
	 * \brief Splits a word into tokens and fills a passed char** with each token
	 *
	 * @param storage is a char** with space to hold LINESIZE sized tokens
	 * @param input is a char[LINESIZE] with @min an empty string and @max a single string of size LINESIZE
	 * 
	 * @return EXIT_SUCCESS if sentence can be tokenized, EXIT_FAILURE if it can't
	 * */

	if(strlen(input) < 1){
		fprintf(stderr, "1.\n%s\n", input);
		return EXIT_FAILURE;
	};
	storage = help_split_line(storage, input);
	return EXIT_SUCCESS;
}
