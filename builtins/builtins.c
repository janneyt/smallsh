
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <ctype.h>
# include <unistd.h>
# include "../signal-project/signal-project.h"


void handle_exit(char args[LINESIZE]) {	

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

