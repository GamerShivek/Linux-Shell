#ifndef REVEAL_H
#define REVEAL_H

#include "lib.h"
#include "command_execution.h"

typedef struct {
    const char *input;
    int pos;
    int length;
} RevParser;

void init_rev_parser(RevParser *p, const char *input);
char rev_peek(RevParser *p);
char rev_consume(RevParser *p);
void rev_skip_whitespace(RevParser *p);
bool parse_rev_name(RevParser *p, char* cwd, bool a_flag, bool l_flag, char* output_file);
bool parse_rev_special_token(RevParser *p, char* cwd, char* shell_homedir, bool a_flag, bool l_flag, char* output_file);
bool parse_rev_flags(RevParser *p, bool *a_flag, bool *l_flag);
bool parse_rev_directory_argument(RevParser *p, char* cwd, char* shell_homedir, bool a_flag, bool l_flag, char* output_file);

bool parse_rev_command(RevParser *p, char* cwd, char* shell_homedir, char* output_file);
void execute_reveal(char *input, char* cwd, char* shell_homedir, char* output_file);
void execute_reveal_with_command_t(command_t *cmd, char *cwd, char *shell_homedir);

#endif