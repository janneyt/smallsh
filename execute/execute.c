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
int handle_redirection(*current) {
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;i
    if (current->input != NULL) {
        input_fd = open(current->input, O_RDONLY);
        if (input_fd == -1) {
            fprintf(stderr, "Failed to open input file %s: %s\n", current->input, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    char *current->output = get_output_file(args);
    if (current->output != NULL) {
        output_fd = open(current->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        if (output_fd == -1) {
            fprintf(stderr, "Failed to open output file %s: %s\n", current->output, strerror(errno));
            if (current->input != NULL) {
                close(input_fd);
            }
            return EXIT_FAILURE;
        }
    }

    if (dup2(input_fd, STDIN_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect input: %s\n", strerror(errno));
        if (current->input != NULL) {
            close(input_nextdnsfd);
        }
        if (current->output != NULL) {
            close(output_fd);
        }
        return EXIT_FAILURE;
    }

    if (dup2(output_fd, STDOUT_FILENO) == -1) {
        fprintf(stderr, "Failed to redirect output: %s\n", strerror(errno));
        if (current->input != NULL) {
            close(input_fd);
        }
        if (current->output != NULL) {
            close(output_fd);
        }
        return EXIT_FAILURE;
    }

    // Execute must be called here to make sure input and output files are closed


    if (current->input != NULL) {
        close(input_fd);
    }

    if (current->output != NULL) {
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
int other_commands(struct ProgArgs current) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    } else if (pid == 0) { // child process 

	if(reset_signals() == EXIT_FAILURE){
		perror("Could not reset signals to smallsh's original signal set");
		return EXIT_FAILURE;
	};

	if(handle_redirection(&current) == EXIT_FAILURE){
		perror("Redirection not possible");
		return EXIT_FAILURE;
	};

        // Execute command
	char** args;
	char* arg = strtok(current->command);
	int index = 0;
	while(arg != NULL){
		args[index] = strtok(current->command);
		index++;
	}
        if(execvp(args[0], args) < 0){
        	perror("");
		fprintf(stderr, "Failed to execute command %s: %s\n", args[0], strerror(errno));
        	exit(EXIT_FAILURE);
	};
    } else { // parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
        return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;
    }
}

int spec_execute(*current){
	char input[LINESIZE];

	strcpy(current->command, "");
	strcpy(current->input, "");
	strcpy(current->output, "");
	strcpy(current->background, false);

	// command is NULL
	if(spec_get_line(input, LINESIZE, stdin) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};

	// command is NULL, but input *something* for command
	if(spec_expansion(input, "$$", 1) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};

	// command is NULL, but now it is being loaded.
	if(spec_parsing(input, &current) == EXIT_FAILURE){
		perror("");
		return EXIT_FAILURE;
	};

	// command is "exit"
	if(strcmp(current->command, "exit") == 0){
		return handle_exit(new_args, current->command);
	};

	// command is "cd"
	else if(current->command[0] == 'c' && current->command[1] == 'd'){
		return execute_cd(current->command[3]);
	}

	// all other commands
	else {
		return other_command(&current);
	}
	


}

