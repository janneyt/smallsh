
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <ctype.h>
# include <unistd.h>
# include <sys/types.h>

# ifndef  LINESIZE
# include "../input/input.h"
# endif

void signal_handler_exit(void){
	// Just ignoring this...
}

void handle_exit(ProgArgs* current, ParentStruct* parent) {	

  	/**
 	* @brief Handles the exit command.
 	* 
 	* @param args The command arguments (not used).
 	*/

	int status;
	
	pid_t pid;
	while((pid = waitpid(getpid(), &status, WNOHANG | WUNTRACED | WCONTINUED) != -1)){
		printf("Pid: %d has status: %d but is ending artificially", pid, status);
		if(kill(SIGKILL, pid) == -1){
			perror("Could not kill all child processes");
			exit(EXIT_FAILURE);
		}
	}



  	// Print the exit message and exit
  	fprintf(stderr, "\nexit\n");
	status = current->command[1] != NULL ? atoi(current->command[1]) : parent->last_foreground;
  	exit(status);
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

