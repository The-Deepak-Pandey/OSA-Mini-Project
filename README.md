# OSA-Mini-Project
Mini project of course - Algorithms and Operating Systems

# Custom C Shell

This project is a custom shell implementation in C, created for an Operating Systems course. It supports basic shell functionalities including built-in commands, foreground and background process execution, I/O redirection, and signal handling.

## Features

-   **Dynamic Prompt:** Displays `<username@system_name:current_directory>`. The shell's startup directory is treated as `~`.
-   **Built-in Commands:**
    -   `echo`: Prints arguments to the console.
    -   `pwd`: Prints the current working directory.
    -   `cd`: Changes the current directory. Supports `~` (home), `-` (previous), and absolute/relative paths.
-   **Command History:**
    -   Persists command history in a `.history` file in the startup directory.
    -   `history`: Displays the last 10 commands. Stores up to 20 commands.
-   **Process Management:**
    -   Execute external commands in the foreground (waits for completion).
    -   Execute external commands in the background using the `&` operator.
-   **I/O Redirection:**
    -   Redirect input using `<`.
    -   Redirect output using `>`.
-   **Signal Handling:**
    -   `Ctrl+C`: Terminates the current foreground process without killing the shell.
    -   `Ctrl+D`: Exits the shell and terminates any running background processes.

## How to Compile and Run

### Prerequisites

-   A C compiler (like `gcc`).
-   `make` utility.

### Compilation and Execution

A `Makefile` is provided for easy compilation. Open your terminal in the project directory and run:

```sh
make all
```

This command will compile all the `.c` source files and create and run an executable file named `a.out`.


You will now see the custom shell prompt and can start entering commands.

### Cleaning Up

To remove the compiled executable and any object files, you can run:

```sh
make clean
```

