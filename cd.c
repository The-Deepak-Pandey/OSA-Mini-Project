#include "cd.h"

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