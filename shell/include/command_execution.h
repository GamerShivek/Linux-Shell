#ifndef COMMAND_EXECUTION_H
#define COMMAND_EXECUTION_H 

#include "lib.h"
#include "globals.h"

// Command structure to hold parsed command information
typedef struct {
    char *args[MAX_ARGS];
    int argc;
    char *input_file;
    char *output_file;
    bool append_output;
} command_t;

// int parse_input_for_args(char *input, char **args);
// void execute_command_bash(char **args);
char* extract_command_name(char *input);
int parse_command_with_redirection(char *input, command_t *cmd);
void execute_command_bash(command_t *cmd);

#endif