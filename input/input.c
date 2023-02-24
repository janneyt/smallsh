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
# ifndef  ParentStruct
# include "../constants/constants.h"

# include "../signal-project/signal-project.h"
# endif
# include <stdlib.h>
# include <errno.h>
# ifndef  spec_execute
# include "../executes/executes.h"
# endif
# ifndef  util_check_environ
# include "../utilities/utilities.h"
# endif

char passed_input[LINESIZE] = {0};
ssize_t passed_size = 0;
int  passed_control_code = 0;
FILE* passed_stream;
int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream, int control_code, ParentStruct* parent);
ParentStruct* passed_parent;

/**
 * @brief Checks for un-waited-for background processes in the same process group ID as smallsh.
 * Prints informative message to stderr for each process and returns EXIT_SUCCESS or EXIT_FAILURE.
 *
 * @return Returns EXIT_SUCCESS if there are no un-waited-for background processes,
 * and EXIT_FAILURE if any child process exited or was signaled.
 */
int spec_check_for_child_background_processes(ParentStruct* parent) {
	pid_t pid;
	int status;
	int exit_status;
	int signaled;

	char hold_pid[LINESIZE];
	int stopped = 0;
	clearerr(stdin);
	clearerr(stdout);
	clearerr(stderr);
    	for(;;) {
        	pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        	if (pid > 0 ) {
			util_int_to_string(pid, hold_pid, 9);
			strcpy(parent->last_background, hold_pid);
        		if (WIFEXITED(status)) {
            			exit_status = WEXITSTATUS(status);
            			fprintf(stderr, "Child process %jd done. Exit status %d.\n", (intmax_t) pid, exit_status);
				fflush(stderr);
				fflush(stdout);
				fflush(stdin);
			} else if (WIFSIGNALED(status)) {
        			signaled = WTERMSIG(status);
        			fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) pid, signaled);
				fflush(stderr);
				fflush(stdout);
				fflush(stdin);

        		} else if (WIFSTOPPED(status)) {
				stopped = WSTOPSIG(status);
				if(stopped == SIGSTOP){
            			if (kill(pid, SIGCONT) == -1) {
                			perror("kill couldn't send a signal");
                    			return EXIT_FAILURE;
                		}
        			fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid);

				}
			}


			return EXIT_SUCCESS;	
		}
		
		if(pid == -1 && errno != 10){
			perror("pid is -1");
			return EXIT_FAILURE;
		} else {
			return EXIT_SUCCESS;
		}
	}
	return EXIT_SUCCESS;
}

void get_line_signal_handler(int signo){
	if(signo != SIGTSTP){
		fprintf(stderr, "\n");
		spec_check_for_child_background_processes(passed_parent);
		fprintf(stderr, "$");

		return;
	}
	
	
}


int spec_get_line(char input[LINESIZE], size_t input_size, FILE* stream, int control_code, ParentStruct* parent){
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
	strcpy(passed_input, input);
	passed_parent = parent;
	passed_size = input_size;
	passed_stream = stream;
	passed_control_code = control_code;
	// input_length currently is zero, as nothing is entered. This would be the same as entering an empty string, so be careful
	ssize_t input_length = 0;

	// This program is constantly setting and resetting errno, so if errno has a value it hasn't been caught elsewhere
	if(errno != 0){
		errno = 0;
	};

	struct sigaction sa = {0};
	struct sigaction oldaction = {0};
	struct sigaction child = {0};
	struct sigaction old_child = {0};
	sa.sa_handler = get_line_signal_handler;
	child.sa_handler = SIG_DFL;
	sa.sa_flags = SA_RESTART;
	child.sa_flags = SA_RESTART;

	sigemptyset(&sa.sa_mask);
	sigemptyset(&child.sa_mask);

	sigaction(SIGINT, &sa, &oldaction);
        sigaction(SIGCHLD, &child, &old_child);

	// PS1 print
	if(control_code == 0){
		fprintf(stderr, "$");
	}

	clearerr(stdin);
	clearerr(stream);
	clearerr(stderr);
	clearerr(stdout);
	errno = 0;
	strcpy(input, "");
	if(( input_length = getline(&input, &input_size, stream)) < 0){				
		sigaction(SIGINT, &oldaction, NULL);
		sigaction(SIGCHLD, &old_child, NULL);
		perror("Problem with errno");
		printf("Input length: %lu\n", input_length);
		fflush(stderr);
		fflush(stdout);
		fflush(stdin);
		if(errno == 10){
			clearerr(stderr);
			clearerr(stdout);
			clearerr(stream);
			clearerr(stdin);
			perror("Inside errno == 10");
			errno = 0;
			return EXIT_SUCCESS;

		}
		if(errno == 11){
			perror("Inside errno == 11");
			clearerr(stderr);
			clearerr(stdout);
			clearerr(stream);
			clearerr(stdin);

			errno = 0;

			return EXIT_SUCCESS;
		}
		perror("other than errno 11 or 10");

		errno = 0;
		
		return EXIT_SUCCESS;
	};
	clearerr(stdin);
	sigaction(SIGINT, &oldaction, NULL);
	sigaction(SIGCHLD, &old_child, NULL);


	if(input_length > LINESIZE-1){
		perror("Input is too large for LINESIZE");
		return EXIT_FAILURE;
	}

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

	char*   old_delim = (strcmp(delim, DELIMITER) == 0) ? DELIMITER : getenv("IFS");
		

	if(bufsize < 1){
		printf("A buffer of 1 or more is needed for tokenization");
		exit(EXIT_FAILURE);
	};
 	
	token = strtok(line, delim);



	while(token != NULL){

		// For bash -c 'exit 166' processing
		if(token[0] == '\'' || token[0] == '\"'){
                        //sigaction(SIGCHLD, &oldaction, NULL);

			// Change delim to look for the closing quote
			delim = "\n\t\'\"";
			token++;			
 			
			char quote_string[LINESIZE] = "";
			strcpy(quote_string, token);
			strcat(quote_string, " ");
			token = strtok(NULL, delim);
			strcat(quote_string, token);
			
			delim = old_delim;
			array_of_tokens[position] = quote_string;
			position++;
			if(token != NULL){
				token = strtok(NULL, delim);
			}

		} else {
		
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
		}
	};

//		        sigaction(SIGCHLD, &oldaction, NULL);

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
