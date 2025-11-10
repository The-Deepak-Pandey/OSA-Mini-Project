#include "echo.h"

void execute_echo(char *args[]){
    // loop through all tokens after "echo" and print them
    for(int i = 1; args[i] != NULL; i++) {
        printf("%s%s", args[i], args[i+1] ? " " : "");
    }
    printf("\n");
}