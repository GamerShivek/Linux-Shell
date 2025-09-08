#ifndef LOG_H
#define LOG_H

#include "lib.h"
#include "globals.h"

typedef struct {
    const char *input;
    int pos;
    int length;
} LogParser;

void init_log_parser(LogParser *p, char *input);
char log_peek(LogParser *p);
char log_consume(LogParser *p);
void log_skip_whitespace(LogParser *p);
void no_extra_command(char* log_file_path);
void purge_logs(char* log_file_path);
void execute_cmd(char* log_file_path, char* command, int n, char* cwd, char* shell_homedir, int cmd_count, int cwd_size, char* output_file);
void check_command(char* command, LogParser *p, char* log_file_path, char* cwd, char* shell_homedir, int cmd_count, int cwd_size, char* output_file);
void execute_log(char* command, FILE* ptr, char* log_file_path, char* cwd, char* shell_homedir, int cmd_count, int cwd_size, char* output_file);
bool command_exists(char** command_list, char* command, int cmd_count);
void add_command_to_log(char* command, char* log_file_path, char command_list[15][512],int* total_cmd_count, int* cmd_count);
bool command_exists_in_buffer(char command_buffer[15][512], char* command, int cmd_count);

#endif
