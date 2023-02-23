# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# ifndef  LINESIZE

# include "../constants/constants.h"
# endif


void spec_signal_handler(void) {
	struct sigaction act;
	struct sigaction child;
	act.sa_handler = SIG_IGN;
	child.sa_flags = SA_RESTART;
	 
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	sigaction(SIGCHLD, &child, NULL);
	sigset_t smallsh_signals;

	sigaddset(&smallsh_signals, SIGINT);
	sigaddset(&smallsh_signals, SIGTSTP);
	sigaddset(&smallsh_signals, SIGCHLD);

}
