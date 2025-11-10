#ifndef GLOBALS_H
#define GLOBALS_H

// 1. SYSTEM INCLUDES
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>

// 2. DEFINES
#define MAX_COMMANDS 20
#define MAX_HISTORY_DISPLAY 10
#define MAX_ARGS 64
#define MAX_LINE_LENGTH 1024

// 3. GLOBAL VARIABLE (Extern) DECLARATIONS
extern char *history[MAX_COMMANDS];
extern int history_count;
extern char previous_dir[MAX_LINE_LENGTH];
extern char home_dir_path[MAX_LINE_LENGTH];
extern pid_t foreground_pid;

#endif // GLOBALS_H