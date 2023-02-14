# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include "../constants/constants.h"

int parent_pid = getppid();

void handle_signal_exit(void){
	// Just ignoring
}

void handle_SIGSTOP(int signo){
	char *message = "Caught SIGSTP: ";
	char *informative = "Entering foreground-only mode, & is ignored";
	char *exiting = "Exiting foreground-only mode, you can use & to send a rpocess to the background";
	char *weird_error = "Something went desperately wrong and the foreground is not in a binary state";
	
	write(STDOUT_FILENO, message, strlen(message));

	if(getppid() == parent_pid){
		printf("%s", message);	
	}
}

void handle_SIGINT(int signo, struct ProgArgs *current){
	char* message = "Caught SIGINT: ";
	int pid = getpid();
	char* str_pid;
	util_int_to_string(pid, str_pid, 10);
	char** exit_args[strlen(str_pid)] = {{0}}

	write(STDOUT_FILENO, message, 39);

	if(current.background){
		write(STDOUT_FILENO, " background ", 200);
		exit_args[0] = "kill";
		exit_args[1] = str_pid;
		
		if(execvp(exit_args[0], exit_args) == -1){
			perror("Could not kill all children");
			exit(EXIT_FAILURE);
		}
		for(int index = 0; index < ){

		}
	}
}

void spec_signal_handler(struct ProgArgs *current) {
	// 
	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};
	SIGINT_action.sa_handler = handle_SIGINT;
	SIGTSTP_action.sa_handler = handle_SIGTSTP;

	sigfillset(&SIGINT_action.sa_mask);
	sigfillset(&SIGTSTP_action.sa_mask);

	SIGIN_action.sa_flags = SA_RESTART;
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	sigset_t smallsh_signals;

	sigaddset(&smallsh_signals, SIGINT);
	

}
