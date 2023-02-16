# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include "../constants/constants.h"

void signal_handler_exit(void);

void handle_SIGSTOP(int signo);
void handle_SIGINT(int signo, ProgArgs *current);
void spec_signal_handler(ProgArgs *current);
