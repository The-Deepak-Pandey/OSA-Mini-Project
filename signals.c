#include "signals.h"
#include "prompt.h"

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
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // find and remove from background_pids
        for (int i = 0; i < background_pid_count; i++) {
            if (background_pids[i] == pid) {
                fprintf(stderr, "\nBackground process %d finished.\n", pid);
                // shift remaining pids to the left
                for (int j = i; j < background_pid_count - 1; j++) {
                    background_pids[j] = background_pids[j+1];
                }
                background_pid_count--;
                break;
            }
        }
    }
    print_shell_prompt();
}