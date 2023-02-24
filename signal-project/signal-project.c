# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# ifndef  LINESIZE

# include "../constants/constants.h"
# endif

void ignore_signal(int signo){
	if(signo == 2 || signo == 9){
		return;
	}
}

void spec_signal_handler(void) {
	struct sigaction act;

	act.sa_handler = ignore_signal;

	 
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);

	sigset_t smallsh_signals;

	sigaddset(&smallsh_signals, SIGINT);
	sigaddset(&smallsh_signals, SIGTSTP);


}
