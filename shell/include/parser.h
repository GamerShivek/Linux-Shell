#ifndef PARSER_H
#define PARSER_H

#include "lib.h"


typedef struct {
    const char *input;
    int pos;
    int length;
} Parser;

void init_parser(Parser *p, const char *input);
char peek(Parser *p);
char consume(Parser *p);
void skip_whitespace(Parser *p);
bool is_name_char(char c);
bool parse_name(Parser *p);
bool parse_input(Parser *p);
bool parse_output(Parser *p);
bool parse_atomic(Parser *p);
bool parse_cmd_group(Parser *p);
bool parse_shell_cmd(Parser *p);
bool is_valid_command(const char *input);
char* extract_command(const char* line);

#endif