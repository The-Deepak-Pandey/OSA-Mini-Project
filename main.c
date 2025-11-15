// Includes all the headers for functions it calls
#include "globals.h"    // Must be first for definitions
#include "signals.h"
#include "history.h"
#include "prompt.h"
#include "execute.h"

// -------------------------------------
// GLOBAL VARIABLE DEFINITIONS
// (These are the actual definitions, not extern)
// -------------------------------------
char *history[MAX_COMMANDS];
int history_count = 0;
char previous_dir[MAX_LINE_LENGTH] = "";
char home_dir_path[MAX_LINE_LENGTH];
pid_t foreground_pid = -1;
// -------------------------------------


int main() {
    // set up signal handlers
    signal(SIGINT, handle_sigint); // handle Ctrl+C
    signal(SIGCHLD, handle_sigchld); // handle terminated child processes
    
    // get home directory path
    const char *home = getenv("HOME");
    if(home == NULL) {
        struct passwd *p = getpwuid(getuid());
        home = p ? p->pw_dir : "";
    } 
    strncpy(home_dir_path, home, sizeof(home_dir_path)-1);
    home_dir_path[sizeof(home_dir_path)-1] = '\0';

    load_history();

    char *line = NULL;
    size_t len = 0;
    while(1) {
        print_shell_prompt();

        if(getline(&line, &len, stdin) == -1) {
            printf("\n");
            break; // exit of ctrl+d
        }

        execute(line);
    }

    free(line);
    save_history();
    // free history memory
    for(int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}