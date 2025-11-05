#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <ctype.h>

#define MAX_COMMANDS 20
#define MAX_HISTORY_DISPLAY 10
#define MAX_ARGS 64
#define MAX_LINE_LENGTH 1024

char *history[MAX_COMMANDS];
int history_count = 0;
char previous_dir[MAX_LINE_LENGTH] = "";
char home_dir_path[MAX_LINE_LENGTH];

void print_shell_prompt() {
    // Get username
    struct passwd *p = getpwuid(getuid());
    const char *username = p ? p->pw_name : "user";
    // printf("Debug: Username: %s\n", username); // Debug line

    // Get hostname
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) != 0){
        strcpy(hostname, "system");
    }
    // printf("Debug: Hostname: %s\n", hostname); // Debug line

    // Get current working directory
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() error");
        return;
    }
    // printf("Debug: Current Working Directory: %s\n", cwd); // Debug line


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
    fflush(stdout);
}

void load_history() {
    FILE *file = fopen("./history.txt", "r");

    if(file == NULL) {
        return; // no history file yet
    }

    char line[MAX_LINE_LENGTH];
    while(fgets(line, sizeof(line), file) != NULL && history_count < MAX_COMMANDS) {
        line[strcspn(line, "\n")] = 0; // remove newline character
        if(strlen(line) > 0) {
            history[history_count] = strdup(line);
            history_count++;
        }
    }
    fclose(file);
}

void save_history() {
    FILE *file = fopen("./history.txt", "w");

    if(file == NULL) {
        perror("Could not open history file for writing");
        return;
    }

    for(int i = 0; i < history_count; i++) {
        fprintf(file, "%s\n", history[i]);
    }
    fclose(file);
}

void add_to_history(const char *command) {
    if(command == NULL || strlen(command) == 0) {
        return; // do not add empty commands
    }

    if(history_count > 0 && strcmp(history[history_count - 1], command) == 0) {
        return; // do not add duplicate consecutive commands
    }

    if(history_count == MAX_COMMANDS) {
        // overwrite the oldest command by shifting everything left
        free(history[0]);
        for(int i = 1; i < MAX_COMMANDS; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_COMMANDS - 1] = strdup(command);
    } else {
        history[history_count] = strdup(command);
        history_count++;
    }
}

void execute_history() {
    int start = (history_count > MAX_HISTORY_DISPLAY) ? (history_count - MAX_HISTORY_DISPLAY) : 0;
    for(int i = start; i < history_count; i++) {
        printf("%s\n", history[i]);
    }
}

void execute_echo(char *args[]){
    // loop through all tokens after "echo" and print them
    for(int i = 1; args[i] != NULL; i++) {
        printf("%s%s", args[i], args[i+1] ? " " : "");
    }
    printf("\n");
}

void execute_pwd() {
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd error");
    }
}

void execute_cd(char *args[]) {
    char curr_dir_before[1024];
    if(getcwd(curr_dir_before, sizeof(curr_dir_before)) == NULL) {
        perror("getcwd() error");
        return;
    }

    char *target_dir;
    // checking 2nd token (args[1])
    if(args[1] == NULL || strcmp(args[1], "~") == 0) {
        target_dir = home_dir_path; // case : cd or cd ~
    } else if (strcmp(args[1], "-") == 0) {
        if(strlen(previous_dir) == 0) {
            fprintf(stderr, "cd: No previous directory yet\n");
            return;
        }
        target_dir = previous_dir; // case : cd -
        printf("%s\n", target_dir); // print the directory we are changing to
    } else {
        target_dir = args[1]; // case : cd <directory>
    }

    if(args[2] != NULL) {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }

    if(chdir(target_dir) != 0) {
        perror("cd error");
    } else {
        // successfully changed directory
        strcpy(previous_dir, curr_dir_before);
    }
}

void execute(char *line) {
    char *args[MAX_ARGS];
    char *token;
    int i = 0;

    char *line_copy = strdup(line); // create a copy for history
    line_copy[strcspn(line_copy, "\n")] = 0; // remove newline character for history

    // Tokenize the input line
    token = strtok(line, " \t\n"); // split by space, tab, newline
    if(token == NULL) { // handle empty or whitespace-only input
        free(line_copy);
        return; // empty input
    }

    add_to_history(line_copy);
    free(line_copy);
    
    while(token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }

    args[i] = NULL; // null-terminate the args array

    if(strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if(strcmp(args[0], "echo") == 0) {
        execute_echo(args);
    } else if(strcmp(args[0], "pwd") == 0) {
        execute_pwd();
    } else if(strcmp(args[0], "cd") == 0) {
        execute_cd(args);
    } else if(strcmp(args[0], "history") == 0) {
        execute_history();
    } else {
        fprintf(stderr, "%s: command not found\n", args[0]);
    }

}

int main() {
    
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
            break; // exit of ctrl+d
        }

        execute(line);

        save_history();
    }

    return 0;
}