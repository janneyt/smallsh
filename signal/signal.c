# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

void handle_SIGINT(int signo){
	
}

void spec_signal_handler(void) {
	// 
	struct sigaction SIGINT_action = {0};

	SIGINT_action.sa_handler = handle_SIGINT;
}
