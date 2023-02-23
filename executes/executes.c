# include "../constants/constants.h"
# include "../utilities/utilities.h"
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h>
# include <signal.h>
# include "../builtins/builtins.h"
# include <string.h>
# include "../parsing/parsing.h"
# include "../expansion/expansion.h"
# include "../input/input.h"
# include <fcntl.h>
# include <sys/stat.h>
# include "../signal-project/signal-project.h"


int spec_execute(ProgArgs* current, FILE* stream, ParentStruct* parent);
int run_commands(ProgArgs* current, ParentStruct* parent);


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
	if(strcmp(current->output, "") != 0 && fileno(output_fd) != STDOUT_FILENO){
		fclose(output_fd);
	}
	input_int = fileno(input_fd);
    } 

    if (strcmp(current->output, "") != 0) {
        output_fd = fopen(current->output, "w+");
        if (output_fd == NULL) {
            fprintf(stderr, "Failed to fopen output file %s: %s\n", current->output, strerror(errno));
            if (strcmp(current->input, "") != 0 && fileno(input_fd) != STDIN_FILENO) {
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

int run_commands(ProgArgs *current, ParentStruct* parent){

	// Processing with a current->command that is not ""
	pid_t wpid = -1;
	pid_t pid = fork();
	int stopped;
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

		
	         
        	if(execvp(current->command[0], current->command) < 0){
			
        		perror("");
			fprintf(stderr, "Failed to execute command %s: %s\n", current->command[0], strerror(errno));
        		exit(EXIT_FAILURE);
		};
		exit(EXIT_SUCCESS);

	} else { // parent process
		int status;
		int hang = 0;

		parent->heap[parent->heap_size] = current;
		parent->heap[parent->heap_size]->pid = pid;
		parent->heap_size++;
		
		// The WNOHANG option sends a process to the background.
		if(current->background){
			hang = WNOHANG;
		}
		if ((wpid = waitpid(0, &status, hang)) == -1) {
            		
			if(errno != 10){
				return EXIT_SUCCESS;
			}
			
	    		return EXIT_FAILURE;
        	}

		
		if(wpid > 0){
			
			if(WIFSTOPPED(status)){
				stopped = WSTOPSIG(status) + 128;
				if(kill(wpid, SIGCONT) == -1){
					return EXIT_FAILURE;
				}
				parent->last_background = stopped;

			} else {
				parent->last_foreground = WEXITSTATUS(status);
			}
			
			parent->heap[parent->heap_size-1] = NULL;
			parent->heap_size--;
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
    	return EXIT_FAILURE;

}


int spec_execute(ProgArgs *current, FILE* stream, ParentStruct* parent){
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
		return EXIT_FAILURE;
	};


	// command is NULL, but input *something* for command
	if(spec_expansion(input, "$$", 1, parent) == EXIT_FAILURE){
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
			if(execute_cd(current->command[1]) == EXIT_FAILURE){
				perror("");
				return EXIT_FAILURE;
			}
		

			return EXIT_SUCCESS;
		}
		return run_commands(current, parent);
	}
	// if there's no command specified BUT input is redirected, still run other_commands in the hope there's a command in the redirected file
	else if(strcmp(current->input, "") != 0 || strcmp(current->output, "") != 0){
		return run_commands(current, parent);
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

