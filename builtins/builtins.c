
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <ctype.h>
# include <unistd.h>
# include <sys/types.h>
# include "../signal-project/signal-project.h"

void signal_handler_exit(void){
	// Just ignoring this...
}

void handle_exit(void) {	

  	/**
 	* @brief Handles the exit command.
 	* 
 	* @param args The command arguments (not used).
 	*/

  	if(kill(0, SIGINT) == -1){
		perror("Could not kill all child processes");
		exit(EXIT_FAILURE);
	};

  	// Print the exit message and exit
  	fprintf(stderr, "\nexit\n");
  	exit(EXIT_SUCCESS);
}

int execute_cd(char *path) {
	/**
 	* @brief Executes the cd (change directory) command with the specified path.
 	* 
 	* @param path The path to change the directory to. If NULL, change to the HOME directory.
 	* 
 	* @return int Returns EXIT_SUCCESS if the directory is changed successfully, EXIT_FAILURE otherwise.
	*/
    	
	// If path is not provided, set it to the HOME directory
    	if (path == NULL) {
        	path = getenv("HOME");

    	}

    	// Change the current directory to the specified path
    	if (chdir(path) != 0) {
        	fprintf(stderr, "Error: Failed to change directory.\n");
        	return EXIT_FAILURE;
    	}

    	return EXIT_SUCCESS;
}

