#ifndef HISTORY_H
#define HISTORY_H

#include "globals.h"

// Public functions from history.c
void load_history();
void save_history();
void add_to_history(const char *command);
void execute_history();

#endif // HISTORY_H