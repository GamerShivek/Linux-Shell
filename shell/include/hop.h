#ifndef HOP_H
#define HOP_H

#include "lib.h"
#include "command_execution.h"

// Get username using getpwuid


typedef struct {
    const char *input;
    int pos;
    int length;
} HopParser;


void init_hop_parser(HopParser *p, const char *input);
char hop_peek(HopParser *p);
char hop_consume(HopParser *p);
void hop_skip_whitespace(HopParser *p);
bool is_hop_name_char(char c);

bool parse_hop_name(HopParser *p, char* cwd);
bool parse_hop_special_token(HopParser *p, char* cwd, char* shell_homedir);
bool parse_hop_argument(HopParser *p, char* cwd, char* shell_homedir);
bool parse_hop_arguments(HopParser *p, char* cwd, char* shell_homedir);
bool parse_hop_command(HopParser *p, char* cwd, char* shell_homedir);

// Command validation and execution
bool is_valid_hop_command(const char *input, char* cwd, char* shell_homedir);
char* execute_hop(char* command, char* cwd, char* shell_homedir);

char* execute_hop_with_command_t(command_t *cmd, char *cwd, char *shell_homedir);

#endif