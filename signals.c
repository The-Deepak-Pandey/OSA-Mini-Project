#include "signals.h"
#include "prompt.h"

void handle_sigint(int sig) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGKILL);
        foreground_pid = -1;
        printf("\n");
    } else {
        printf("\n");
        print_shell_prompt();
    }
}

// to clean up zombie processes from background processes
void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    int re_prompt = 0;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // find and remove from background_pids
        for (int i = 0; i < background_pid_count; i++) {
            if (background_pids[i] == pid) {
                fprintf(stderr, "Background process %d finished.\n", pid);
                re_prompt = 1;
                // shift remaining pids to the left
                for (int j = i; j < background_pid_count - 1; j++) {
                    background_pids[j] = background_pids[j+1];
                }
                background_pid_count--;
                break;
            }
        }
    }
    if (re_prompt) {
        print_shell_prompt();
    }
}