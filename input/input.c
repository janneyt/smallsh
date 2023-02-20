# define _GNU_SOURCE
# include <signal.h>
# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>
#ifndef LINESIZE
# include "../constants/constants.h"
#endif
# ifndef  handle_signal_exit
# include "../execute/execute.h"
# include "../signal-project/signal-project.h"
# endif
# include <stdlib.h>
# include <errno.h>
# ifndef  heap_size
# include "../heap/heap.h"
# endif
# ifndef  util_check_environ
# include "../utilities/utilities.h"
# endif


/**
 * @brief Checks for un-waited-for background processes in the same process group ID as smallsh.
 * Prints informative message to stderr for each process and returns EXIT_SUCCESS or EXIT_FAILURE.
 *
 * @return Returns EXIT_SUCCESS if there are no un-waited-for background processes,
 * and EXIT_FAILURE if any child process exited or was signaled.
 */
int spec_check_for_child_background_processes(ParentStruct *parent) {
	pid_t pid;
	int status;
	int exit_status;
	int signaled;
	int stopped;

    	for(;;) {
        	pid = waitpid(-getpid(), &status, WNOHANG | WUNTRACED | WCONTINUED);
        	if (pid != -1 && pid != 0) {
        

        		if (WIFEXITED(status)) {
            			exit_status = WEXITSTATUS(status);
            			fprintf(stderr, "Child process %d done. Exit status %d.\n", pid, exit_status);
        		} else if (WIFSIGNALED(status)) {
        			signaled = WTERMSIG(status);
        			fprintf(stderr, "Child process %d done. Signaled %d.\n", pid, signaled);
        		} else if (WIFSTOPPED(status)) {
        			stopped = WSTOPSIG(status);
        			if (stopped == SIGSTOP) {
        				if (kill(pid, SIGCONT) == -1) {
                				perror("kill couldn't send a signal");
                    				return EXIT_FAILURE;
                			}
                			fprintf(stderr, "Child process %d stopped. Continuing.\n", pid);
            			}
			}

			ProgArgs* current = parent->heap[find_in_heap(pid, parent->heap)];
			// The specifcations require us to track the latest exited process for both background and foreground	
			if(current->background){
				parent->last_background = exit_status;
			} else {
				parent->last_foreground = exit_status;
			}
		}
	if(pid == -1 && errno != 10){

		return EXIT_FAILURE;
	} else if(pid == -1 && errno == 10){

		return EXIT_SUCCESS;
	}
	if(pid == 0){
		return EXIT_SUCCESS;
	}
	}
	return EXIT_SUCCESS;
}


int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream, int control_code){
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
	struct sigaction sa;
	sa.sa_handler = handle_signal_exit;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// PS1 print
	if(control_code == 0){
		printf("$");
	}
	
	if(( input_length = getline(&input, &input_size, stream)) < 0){
		perror("Cannot fetch line from input");
		clearerr(stream);
		errno = 0;
		return EXIT_FAILURE;
	};

	assert(input_length >= 0);
	reset_signals();
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
	char*	delim = DELIMITER;
	char    ifs[LINESIZE] = "IFS";
	if(util_check_environ(ifs)){
		delim = getenv("IFS");
	}
	int	token_bufsize = 64;
	char**  array_of_tokens = storage;

		
	if(bufsize < 1){
		printf("A buffer of 1 or more is needed for tokenization");
		exit(EXIT_FAILURE);
	};
	
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
