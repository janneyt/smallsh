# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# ifndef  LINESIZE

# include "../constants/constants.h"
# endif
# ifndef  spec_check_for_child_background_processes
# include "../input/input.h"
# endif

ParentStruct passed_parent;

void handle_sigint(int signo){
	if(signo == SIGINT){
		return;
	}
}

void spec_signal_handler(ParentStruct* parent) {
	passed_parent = *parent;
	struct sigaction act;
	struct sigaction handle_sigtstp;
	struct sigaction oldaction;
	handle_sigtstp.sa_handler = SIG_IGN;
	act.sa_handler = handle_sigint;
	sigfillset(&handle_sigtstp.sa_mask);
	sigfillset(&act.sa_mask);
	handle_sigtstp.sa_flags = 0;
	act.sa_flags = 0;
	
	 
	sigaction(SIGINT, &act, &oldaction);
	sigaction(SIGTSTP, &handle_sigtstp, &oldaction);
	parent->oldaction = oldaction;
	sigset_t smallsh_signals;

	sigaddset(&smallsh_signals, SIGINT);
	sigaddset(&smallsh_signals, SIGTSTP);


}
