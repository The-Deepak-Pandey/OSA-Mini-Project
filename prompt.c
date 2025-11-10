#include "prompt.h"

void print_shell_prompt() {
    // Get username
    struct passwd *p = getpwuid(getuid());
    const char *username = p ? p->pw_name : "user";

    // Get hostname
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) != 0){
        strcpy(hostname, "system");
    }

    // Get current working directory
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() error");
        return;
    }

    // Check if current directory is within home directory
    char display_dir[1024];
    if(strlen(home_dir_path) > 0 && strstr(cwd, home_dir_path) == cwd){
        // if it is, replace home directory with '~'
        snprintf(display_dir, sizeof(display_dir), "~%s", cwd + strlen(home_dir_path));
    } else {
        // else use the full path
        strncpy(display_dir, cwd, sizeof(display_dir));
    }

    // print the formatted prompt
    printf("<%s@%s:%s> ", username, hostname, display_dir);
    fflush(stdout); // ensure prompt is displayed immediately
}