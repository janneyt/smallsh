# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif


void handle_signal_exit(int signo){
	// Just ignoring
	printf("%d", signo);
}

void handle_SIGSTOP(int signo){
	printf("%d", signo);
}

void handle_SIGINT(int signo){
	printf("%d", signo);
}

void spec_signal_handler(void) {
	// 
	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};
	SIGINT_action.sa_handler = handle_SIGINT;
	SIGTSTP_action.sa_handler = handle_SIGSTOP;

	sigfillset(&SIGINT_action.sa_mask);
	sigfillset(&SIGTSTP_action.sa_mask);

	SIGINT_action.sa_flags = SA_RESTART;
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	sigset_t smallsh_signals;

	sigaddset(&smallsh_signals, SIGINT);
	

}
