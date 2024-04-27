/**************************************************************
* Class:  CSC-415-01 Summer 2023
* Name: Oscar Galvez
* Student ID: 911813414
* GitHub Name: gojilikeog
* Project: Assignment 3 – Simple Shell
*
* File: Galvez_Oscar_HW3_main.cpp
*
* Description: This assignment is to implement a simple shell that
* runs on top of the regular command-line interpreter. It is to
* accept user input, parse the input, then execute the command
* by creating a new process. It is to exit if the user specifies
* to exit the shell.
*
**************************************************************/

#include <stdio.h> // Input and output operations - Pretty sure needed
#include <stdlib.h> // Type conversion - memory allocation - determine if needed
#include <string.h> // String handling
#include <sys/types.h> // Needed to use pid_t
#include <sys/wait.h> // Needed to use wait() - ANTICIPATING USE
#include <unistd.h> // Enables fork function - execvp()

// Implemented for ease of modification
#define CHARACTER_BUFFER 116
#define MAX_COMMANDS 58

int main(int argc, char *argv[]) {

    // Must accept a command line argument which is a prefix prompt
    // If not specificed, use the '>' as the prompt - COMPLETED
    char prefixPrompt = '>';
    char userCommand[CHARACTER_BUFFER];
    char *arguments[MAX_COMMANDS];

    if (argc > 1) {
        printf("%s ", argv[1]);
    } else {
        printf("%c ", prefixPrompt);
    }

    // Do not overflow a 116 byte buffer when fetching input
    // COMPLETED - DOESN'T ACCEPT MORE THAN 115 CHAR IGNORES REST
    // fgets has a size limit - Use it
    fgets(userCommand, CHARACTER_BUFFER, stdin);

    // fgets accepts no input as a new line character
    // If the user entered an empty line, report the error and field
    // a new line of input

    while (userCommand[0] == '\n') {
        if (argv[1] == NULL) {
            printf ("No input recorded. Please try again\n%c ", prefixPrompt);
        } else {
            printf ("No input recorded. Please try again\n%s ", argv[1]);
        }

        fgets(userCommand, 116, stdin);
    }

    // Trailing new line character from using fgets - Removed with this
    // line after checks are completed.
    userCommand[strlen(userCommand)-1] = '\0';
    
    // Store pointers to the substrings in an array (similar to argv passed into main)
    // Pass this into execvp()
    // Number of resulting tokens cannot be known prior
    // GIVEN AS A HINT FOR THE ASSIGNMENT - Breaks the string 'str'
    // into a series of tokens using the delimiter 'delim'
    int i = 0;

    char *token = strtok(userCommand, " ");
    while (token != NULL ) {

        // REQUIREMENT:
        // If user enters the exit command, your shell should terminate
        if (strcmp(arguments[0], "exit") == 0) {
            exit(0);
        }

        arguments[i++] = token;
        token = strtok(NULL, " ");
    }

    // REQUIREMENT:
    // Null pointer after the last substring
    if (token == NULL)
    {
        arguments[i] == token;
    }

    pid_t pid = fork();
    char* command = arguments[0];

    // TODO
    // while (1) CONTINUES UNTIL BROKEN
    // enclose all code within while loop where appropriate 
    if (pid < 0) {
        printf("ERROR: Fork failed.\n");
        exit(1);
    } else {
        if (pid == 0) {
        printf("I am the child - my command is %s\n", arguments[0]);
        if (execvp(arguments[0], arguments) < 0) {
            printf("ERROR: exec failed\n");
            exit(1);
        } else {
            wait(0);
            printf("I am the parent\n");
            printf("Child PID: %d\n", pid);
            exit(0);
            }
        }
    }
    
    return 0;
}
