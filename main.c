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

#define MAX_COMMANDS 20
#define MAX_HISTORY_DISPLAY 10
#define MAX_ARGS 64
#define MAX_LINE_LENGTH 1024

char *history[MAX_COMMANDS];
int history_count = 0;
char previous_dir[MAX_LINE_LENGTH] = "";
char home_dir_path[MAX_LINE_LENGTH];
pid_t foreground_pid = -1; // pid of the current foreground process

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
    fflush(stdout); // ensure prompt is displayed immediately
}

void load_history() {
    char history_path[MAX_LINE_LENGTH]; 
    
    // Use sizeof(history_path) to safely tell snprintf the buffer size
    snprintf(history_path, sizeof(history_path), "%s/.historyy", home_dir_path);
    
    FILE *file = fopen(history_path, "r");

    if(file == NULL) {
        return; // no history file yet
    }

    char line[MAX_LINE_LENGTH]; // This is fine, reading a line *from* the file
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
    char history_path[MAX_LINE_LENGTH]; 
    
    // Use sizeof(history_path) to safely tell snprintf the buffer size
    snprintf(history_path, sizeof(history_path), "%s/.historyy", home_dir_path);
    
    FILE *file = fopen(history_path, "w");

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
    int background = 0;
    char *input_file = NULL;
    char *output_file = NULL;

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
        if (strcmp(token, "<") == 0) { // input redirection
            token = strtok(NULL, " \t\n");
            if(token) input_file = token;
        } else if (strcmp(token, ">") == 0) { // output redirection
            token = strtok(NULL, " \t\n");
            if(token) output_file = token;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }

    args[i] = NULL; // null-terminate the args array

    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1; // background execution
        args[i - 1] = NULL; // remove '&' from args
    }

    if (args[0] == NULL) {
        return; // no command to execute
    }

    // 1. Handle built-ins that MUST run in the parent process
    if(strcmp(args[0], "exit") == 0) {
        save_history();
        exit(0);
    } else if(strcmp(args[0], "cd") == 0) {
        execute_cd(args);
        return; // 'cd' is done, return to main loop
    }

    // 2. Fork for all other commands (built-ins and external)
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork error");
        return;
    } 
    
    if (pid == 0) {
        // --- CHILD PROCESS ---

        // 3. Set up I/O redirection (happens for all child commands)
        if (input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("Open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO); // redirect stdin
            close(fd_in);
        }
        if (output_file) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("Open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO); // redirect stdout
            close(fd_out);
        }

        // 4. Check for child-run built-ins
        if(strcmp(args[0], "echo") == 0) {
            execute_echo(args);
            exit(EXIT_SUCCESS); // Done, exit child
        } else if(strcmp(args[0], "pwd") == 0) {
            execute_pwd();
            exit(EXIT_SUCCESS); // Done, exit child
        } else if(strcmp(args[0], "history") == 0) {
            execute_history();
            exit(EXIT_SUCCESS); // Done, exit child
        } else {
            // 5. Not a built-in, run external command
            if (execvp(args[0], args) == -1) {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(EXIT_FAILURE);
            }
        }

    } else {
        // --- PARENT PROCESS ---
        if (background) {
            printf("Started backgound process [PID: %d]\n", pid); // print background process PID
        } else {
            foreground_pid = pid; // set the foreground process PID
            int status;
            waitpid(pid, &status, 0); // wait for the foreground process to finish
            foreground_pid = -1; // reset foreground PID
        }
    }
}

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