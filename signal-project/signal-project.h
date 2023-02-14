# include <signal.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include "../constants/constants.h"

void handle_signal_exit(void);

void handle_SIGSTOP(int signo);
void handle_SIGINT(int signo, struct ProgArgs *current);
void spec_signal_handler(struct ProgArgs *current);
