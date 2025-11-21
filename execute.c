// Includes its own header AND headers for functions it calls
#include "execute.h"
#include "history.h"
#include "cd.h"
#include "echo.h"
#include "pwd.h"

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
        if (background) {
            setpgid(0, 0); // this 
        }
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
            printf("Started background process [PID: %d]\n", pid);
            // Add to background process list
            if (background_pid_count < MAX_COMMANDS) {
                background_pids[background_pid_count++] = pid;
            }
        } else {
            foreground_pid = pid; // Set global foreground pid
            int status;
            waitpid(pid, &status, 0); // wait for the foreground process to finish
            foreground_pid = -1; // reset foreground PID
        }
    }
}
