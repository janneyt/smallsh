
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
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    if (strcmp(current->input, "") != 0) {
        input_fd = open(current->input, O_RDONLY);
        if (input_fd == -1) {
            fprintf(stderr, "Failed to open input file %s: %s\n", current->input, strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
	return EXIT_SUCCESS;
    }

    if (strcmp(current->output, "") != 0) {
        output_fd = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        if (output_fd == -1) {
            fprintf(stderr, "Failed to open output file %s: %s\n", current->output, strerror(errno));
            if (strcpy(current->input, "") == 0) {
                close(input_fd);
            }
            return EXIT_FAILURE;
        }
    } else {
	return EXIT_SUCCESS;
    }
    

    if (dup2(input_fd, STDIN_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect input: %s\n", strerror(errno));
        if (strcpy(current->input, "") == 0) {
            close(input_fd);
        }
        if (strcpy(current->output, "") == 0) {
            close(output_fd);
        }
        return EXIT_FAILURE;
    }

    if (dup2(output_fd, STDOUT_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect output: %s\n", strerror(errno));
        if (strcpy(current->input, "") != 0) {
            close(input_fd);
        }
        if (strcpy(current->output,"") != 0) {
            close(output_fd);
        }
        return EXIT_FAILURE;
    }

    // Execute must be called here to make sure input and output files are closed


    if (strcpy(current->input,"") == 0) {
        close(input_fd);
    }

    if (strcpy(current->output, "") == 0) {
        close(output_fd);
    }

    return EXIT_SUCCESS;
}



/**
 * @brief Executes a command using fork and execvp.
 * 
 * @param cmd The command to execute.
 * @return EXIT_SUCCESS if the command was executed successfully, EXIT_FAILURE otherwise.
 */
int other_commands(ProgArgs *current) {
	printf("in other commans 1\n");
	fflush(stdout);
	fflush(stderr);
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed. Reason for failure:");
        return EXIT_FAILURE;
    } else if (pid == 0) { // child process 
	perror("");
	clearerr(stdout);
	clearerr(stderr);
	if(reset_signals() == EXIT_FAILURE){
		perror("Could not reset signals to smallsh's original signal set");
		return EXIT_FAILURE;
	};

	if(handle_redirection(current) == EXIT_FAILURE){
		perror("Redirection not possible");
		return EXIT_FAILURE;
	};
	
        
        if(execvp(current->command[0], current->command) < 0){
        	perror("");
		fprintf(stderr, "Failed to execute command %s: %s\n", current->command[0], strerror(errno));
        	exit(EXIT_FAILURE);
	};
	printf("Executed!\n");
	fflush(stdout);
	fflush(stderr);
	clearerr(stdout);
	clearerr(stderr);
    } else { // parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
	perror("status: \n");
        return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int spec_execute(ProgArgs *current){
	char input[LINESIZE];
	strcpy(current->input, "");
	strcpy(current->output, "");
	current->background = false;
	printf("in spec_execute 1\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");
	// command is NULL
	if(spec_get_line(input, LINESIZE, stdin) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};
	printf("in spec_execute 2\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");
	// command is NULL, but input *something* for command
	if(spec_expansion(input, "$$", 1) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};

	printf("in spec_execute 3\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");

	// command is NULL, but now it is being loaded.
	if(spec_parsing(input, current) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};


	printf("in spec_execute 4\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");



	// command is "exit"
	if(strcmp(current->command[0], "exit") == 0){
		handle_exit();
		
	printf("in spec_execute 5\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");


	}


	// command is "cd"
	else if(current->command[0][0] == 'c' && current->command[0][1] == 'd'){
		if(execute_cd(current->command[3]) == EXIT_FAILURE){
			perror("");
			return EXIT_FAILURE;
		}
		
	printf("in spec_execute 6\n");
	fflush(stdout);
	fflush(stderr);
	getenv("IFS");


		return EXIT_SUCCESS;
	}


	// all other commands
	else {
		printf("in spec_execute before other commands \n");
		fflush(stdout);
		fflush(stderr);
		return other_commands(current);
	}

	return EXIT_SUCCESS;
}

