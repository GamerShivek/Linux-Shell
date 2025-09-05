#ifndef PIPING_H
#define PIPING_H

#include "lib.h"
#include "command_execution.h"

int parse_pipeline(char *input, command_t cmds[], int *num_cmds);
void setup_redirection(command_t *cmd);
void execute_pipeline(command_t cmds[], int num_cmds, char* cwd, char* shell_homedir);

#endif