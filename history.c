#include "history.h"

void load_history() {
    char history_path[MAX_LINE_LENGTH]; 
    
    snprintf(history_path, sizeof(history_path), "%s/.historyy", home_dir_path);
    
    FILE *file = fopen(history_path, "r");

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
    char history_path[MAX_LINE_LENGTH]; 
    
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