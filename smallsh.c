# include <stdio.h>
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdint.h>

int int_to_string(int num, char* str, int size){
	// Converts an integer to its string representation
	char temp[size];
	sprintf(temp, "%d", num);
	if(strlen(str) < strlen(temp)){
		return EXIT_FAILURE;
	}
	if(strcat(str, temp) == NULL){
		perror("Could not convert integer to string");
		return EXIT_FAILURE;
	};
	return EXIT_SUCCESS;
}

int check_for_child_background_processes(int status, int pid){

	if (WIFEXITED(status)){
		if(fprintf(stderr, "Child process %jd done. Exit status %d.\n", (intmax_t) pid, WEXITSTATUS(status)) < 0){
			perror("Could not print");
			return EXIT_FAILURE;
		};

	}
	else if (WIFSIGNALED(status)){
		if(fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) pid, WTERMSIG(status)) < 0){
			perror("Could not print");
			return EXIT_FAILURE;
		};
		
	}
	else if (WIFSTOPPED(status)) {
		if(kill(pid, SIGCONT) != 0){
			perror("Kill didn't work");
			return EXIT_FAILURE;
		}
		if(fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid) < 0){
			perror("Could not print");
			return EXIT_FAILURE;
		};

	}
	return EXIT_SUCCESS;
}

int main(){

	pid_t pid;
	int status;
	
	for(;;){

		// Specification requirement to identify child processes potentially exiting between loops
		while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			if(check_for_child_background_processes(status, pid) != EXIT_SUCCESS){
				exit(EXIT_FAILURE);
			}
		}

		// Print prompt
	}


	exit(EXIT_SUCCESS);
}
