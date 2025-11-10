#include "signals.h"

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) { 
    if(foreground_pid > 0) {
        kill(foreground_pid, SIGINT); // terminate the foreground child process
        foreground_pid = -1;
    }
    printf("\n"); // move to new line after Ctrl+C
}

// to clean up zombie processes from background processes
void handle_sigchld(int sig) {
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0) {
        // this loop reaps all terminated child processes
    }
}