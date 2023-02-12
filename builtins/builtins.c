# include "exit_handler.h"
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <ctype.h>
# include <unistd.h>
# include "../signal-project/signal-project.h"


void handle_exit(int num_args, char* args[]) {
	int exit_status;

  	// Parse the arguments
  	if (argc > 2) {
    		fprintf(stderr, "Usage: %s [exit_status]\n", argv[0]);
    		exit_status = EXIT_FAILURE;	
	
  	} else if (argc == 2) {
    		if (!isdigit(argv[1][0])) {
      			fprintf(stderr, "%s: argument must be an integer\n", argv[0]);

    		}
    		exit_status = atoi(argv[1]);
  	} else {
    		// No argument provided, use the exit status of the last command
    		exit_status = WEXITSTATUS(status);
  	}

  	// Send SIGINT to all child processes
  	signal(SIGINT, signal_handler_exit);
  	kill(0, SIGINT);

  	// Print the exit message and exit
  	fprintf(stderr, "\nexit\n");
  	exit(exit_status);
}

int execute_cd(char *path) {
    // If path is not provided, set it to the HOME directory
    if (path == NULL) {
        path = getenv("HOME");
        if (path == NULL) {
            fprintf(stderr, "Error: HOME directory not found.\n");
            return EXIT_FAILURE;
        }
    }

    // Change the current directory to the specified path
    if (chdir(path) != 0) {
        fprintf(stderr, "Error: Failed to change directory.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

