
# ifndef  LINESIZE
# include "../constants/constants.h"
# include "../utilities/utilities.h"
# endif
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h>
# include <signal.h>
# include "../builtins/builtins.h"
# include <string.h>
# include "../parsing/parsing.h"
# include "../expansion/expansion.h"
# ifndef  handle_signal_exit
# include "../input/input.h"
# endif
# include <fcntl.h>
# include <sys/stat.h>

int spec_execute(ProgArgs* current, FILE* stream);
int run_commands(ProgArgs* current);

/**
 * @brief Resets all signals to their default disposition.
 *
 * @return EXIT_SUCCESS if all signals were reset successfully, EXIT_FAILURE otherwise.
 */
int reset_signals() {
	// Reset all signals to their original dispositions
        struct sigaction oldact;
        if(sigaction(SIGINT, NULL, &oldact) == -1){
		perror("Cannot reset SIGINT");
		return EXIT_FAILURE;
	};
        if (oldact.sa_handler != SIG_DFL) {
            if(sigaction(SIGINT, &oldact, NULL) == -1){
		perror("Cannot reset SIGINT as SIG_DFL does not equal oldact.sa_handler");
		return EXIT_FAILURE;
	    };

        }
        return EXIT_SUCCESS;
}




/**
 * @brief Handles input and output redirection.
 *
 * @param args The command arguments.
 * @return EXIT_SUCCESS if redirection was successful, EXIT_FAILURE otherwise.
 */

int handle_redirection(ProgArgs *current) {
    FILE* input_fd = stdin;
    FILE* output_fd = stdout;
    int input_int = STDIN_FILENO;
    int output_int = STDOUT_FILENO;
    if (strcmp(current->input, "") != 0) {
        input_fd = fopen(current->input, "r+");
        if (input_fd == NULL) {
            fprintf(stderr, "Failed to fopen input file %s: %s\n", current->input, strerror(errno));
            return EXIT_FAILURE;
        }
	if(strcmp(current->output, "") != 0){
		fclose(output_fd);
	}
	input_int = fileno(input_fd);
    } 

    if (strcmp(current->output, "") != 0) {
        output_fd = fopen(current->output, "w+");
        if (output_fd == NULL) {
            fprintf(stderr, "Failed to fopen output file %s: %s\n", current->output, strerror(errno));
            if (strcmp(current->input, "") != 0) {
                fclose(input_fd);
            }
            return EXIT_FAILURE;
        }
	output_int = fileno(output_fd);
    } 

    if(strcmp(current->output, "") == 0 && strcmp(current->input, "") == 0){
	return EXIT_SUCCESS;
    }

    if (dup2(input_int, STDIN_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect input: %s\n", strerror(errno));
        if (strcmp(current->input, "") == 0 || fclose(input_fd) == EOF){
		return EXIT_FAILURE;
        }
        if (strcmp(current->output, "") == 0 || fclose(output_fd) == EOF){
		return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    } else if(strcmp(current->input,"") != 0) {

	// Only choose this option if input is redirected
	// The newly acquired input from the non-stdin file has to be processed
	char file_input[LINESIZE];
	if(spec_get_line(file_input, LINESIZE, input_fd, 1) == EXIT_FAILURE || 
			spec_expansion(file_input, "$$", 1) == EXIT_FAILURE ||
			spec_parsing(file_input, current) == EXIT_FAILURE){
		perror("Could not access a line from the redirected input");
		return EXIT_FAILURE;
	}
    }

    if (dup2(output_int, STDOUT_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect output: %s\n", strerror(errno));
        if (strcmp(current->input, "") == 0 || fclose(input_fd) == EOF){
		return EXIT_FAILURE;
        }
        if (strcmp(current->output,"") == 0 || fclose(output_fd) == EOF){
		return EXIT_FAILURE;
        }
        return EXIT_FAILURE;
    }


    // Handle redirection only occurs in a child process, so execute the process here
    if(strcmp(current->command[0], "") != 0 && execvp(current->command[0], current->command) < 0){
	    
	perror("\n");
	fprintf(stderr, "Failed to execute command %s: %s\n", current->command[0], strerror(errno));
	strcpy(current->command[0], "");
	strcpy(current->input, "");
	strcpy(current->output, "");
	current->background = false;
        if(dup2(STDIN_FILENO, input_int) < 0){
		perror("Can't redirect back to stdin");
		if( output_int != STDOUT_FILENO || fclose(output_fd) == EOF){
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
    	}
    	if(dup2(STDOUT_FILENO, output_int) < 0){
		perror("Can't redirect back to stdout");
		if(input_int != STDIN_FILENO || fclose(output_fd) == EOF){
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
    	};

    	if(input_int != STDIN_FILENO && fclose(input_fd) == EOF){
		return EXIT_FAILURE;
    	};
    	if(output_int != STDOUT_FILENO && fclose(output_fd) == EOF){
		return EXIT_FAILURE;
    	};
    	strcpy(current->input, "");
    	strcpy(current->output, "");
	return EXIT_FAILURE;
    };

	strcpy(current->command[0], "");
	strcpy(current->input, "");
	strcpy(current->output, "");
	current->background = false;
        if(dup2(STDIN_FILENO, input_int) < 0){
		perror("Can't redirect back to stdin");
		if( output_int != STDOUT_FILENO || fclose(output_fd) == EOF){
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
    	}
    	if(dup2(STDOUT_FILENO, output_int) < 0){
		perror("Can't redirect back to stdout");
		if(input_int != STDIN_FILENO || fclose(output_fd) == EOF){
			return EXIT_FAILURE;
		}
		return EXIT_FAILURE;
    	};

    if(fileno(input_fd) != STDIN_FILENO && fclose(input_fd) == EOF){
	    perror("Could not close input_fd");
	    return EXIT_FAILURE;
    };
    if(fileno(output_fd) != STDOUT_FILENO && fclose(output_fd) == EOF){
	    perror("Could not close output_fd");
	    return EXIT_FAILURE;
    };
    strcpy(current->input, "");
    strcpy(current->output, "");
    return EXIT_SUCCESS;
}

int run_commands(ProgArgs *current){



	// Processing with a current->command that is not ""
	pid_t pid = fork();
    	if (pid == -1) {
        	perror("Fork failed. Reason for failure:");
        	return EXIT_FAILURE;
    	} else if (pid == 0) { // child process 

		// So far, I've tolerated a missing command, but only because both
		// redirection options have a pseudo command built in. However, all 
		// other alternatives must be discarded

		if( strcmp(current->command[0], "") == 0 && (strcmp(current->input, "") != 0 || strcmp(current->output, "") != 0)){
			if(handle_redirection(current) == EXIT_FAILURE){
				
				perror("Redirection problem");
				if(strcmp(current->command[0], "") != 0){
					strcpy(current->command[0], "");
				}
				strcpy(current->input, "");
				strcpy(current->output, "");
				current->background = false;
				
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		} else if(strcmp(current->command[0], "") == 0){ 
			exit(EXIT_FAILURE);
		} else if(strcmp(current->input, "") != 0 || strcmp(current->output, "") != 0){
			if(handle_redirection(current) == EXIT_FAILURE){
				perror("Redirection problem");
				if(strcmp(current->command[0], "") != 0){
					strcpy(current->command[0], "");
				}
				strcpy(current->input, "");
				strcpy(current->output, "");
				current->background = false;
				exit(EXIT_FAILURE);
			}
		}

		if(reset_signals() == EXIT_FAILURE){
			perror("Could not reset signals to smallsh's original signal set");
			exit(EXIT_FAILURE);
		};

	         
        	if(execvp(current->command[0], current->command) < 0){
        		perror("");
			fprintf(stderr, "Failed to execute command %s: %s\n", current->command[0], strerror(errno));
        		exit(EXIT_FAILURE);
		};
		exit(EXIT_SUCCESS);

	} else { // parent process
		int status;
		int hang = 0;
		
		// The WNOHANG option sends a process to the background.
		if(current->background){
			hang = WNOHANG;
		}
		if (waitpid(0, &status, hang) == -1) {
            		perror("Waiting error: ");
	    		return EXIT_FAILURE;
        	}


		// Resetting the other options (for the next loop of the parent) is more straightforward
		if(strcmp(current->command[0], "") != 0){
			strcpy(current->command[0], "");
		}
		strcpy(current->input, "");
		strcpy(current->output, "");
		current->background = false;
        	return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_SUCCESS;
    	}
	perror("\n");
    	return EXIT_FAILURE;

}

/**
 * @brief Executes a command using fork and execvp.
 * 
 * @param cmd The command to execute.
 * @return EXIT_SUCCESS if the command was executed successfully, EXIT_FAILURE otherwise.
 */
int other_commands(ProgArgs *current) {

	return run_commands(current);

}

int spec_execute(ProgArgs *current, FILE* stream){
	char input[LINESIZE];

        if(current->command[0] == NULL){
		current->command[0] = '\0';
		strcpy(current->command[0], "");
	}	
	strcpy(current->input, "");
	strcpy(current->output, "");
	current->background = false;
	// command is NULL
	if(spec_get_line(input, LINESIZE, stream, 0) == EXIT_FAILURE){
		perror("Spec get line errored:");
		return EXIT_FAILURE;
	};


	// command is NULL, but input *something* for command
	if(spec_expansion(input, "$$", 1) == EXIT_FAILURE){
		perror("Spec expansion errored:");
		return EXIT_FAILURE;
	};


	// command is NULL, but now it is being loaded.
	if(spec_parsing(input, current) == EXIT_FAILURE){
		perror("Spec parsing errored:");
		return EXIT_FAILURE;
	};

	// Built in commands are only possible when the command function is loaded, but if only input redirection is listed then 
	// there is no command on the first run through
	if(strcmp(current->command[0], "") != 0){
		// command is "exit"
		if(strcmp(current->command[0], "exit") == 0){
			handle_exit();
		

		}


		// command is "cd"
		else if(current->command[0][0] == 'c' && current->command[0][1] == 'd'){
			if(execute_cd(current->command[3]) == EXIT_FAILURE){
				perror("");
				return EXIT_FAILURE;
			}
		

			return EXIT_SUCCESS;
		}
		return other_commands(current);
	}
	// if there's no command specified BUT input is redirected, still run other_commands in the hope there's a command in the redirected file
	else if(strcmp(current->input, "") != 0 || strcmp(current->output, "") != 0){
		return other_commands(current);
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

