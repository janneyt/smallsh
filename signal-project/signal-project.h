# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif

int reset_signals(void);
void handle_signal_exit(int signo);

void handle_SIGSTOP(int signo);
void handle_SIGINT(int signo, ProgArgs *current);
void spec_signal_handler(ProgArgs *current);
