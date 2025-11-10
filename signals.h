#ifndef SIGNALS_H
#define SIGNALS_H

#include "globals.h"

// Public functions from signals.c
void handle_sigint(int sig);
void handle_sigchld(int sig);

#endif // SIGNALS_H