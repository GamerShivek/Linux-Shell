#ifndef BG_H
#define BG_H

#include "lib.h"
#include "globals.h"
#include "command_execution.h"

typedef struct {
    pid_t pid;
    int job_num;
    char cmd_name[1024];
    int active;
    int status; // status : 1 for running, 0 for stopped
} bg_process_t;

#define MAX_BG_PROCESSES 64

void parse_command(char *cmd, char *argv[]);
void send_to_background(command_t *cmd, char *curr_command, int status);
void check_background_processes();
void execute_custom_fnc_in_bg(command_t *cmd, char *cwd, char *shell_homedir, char *current_command, char* log_file_path, int cmd_count);
void execute_pipeline_in_bg(command_t cmds[], int num_cmds, char *cwd, char *shell_homedir, char* current_command);
bg_process_t* get_job_list();
int get_num_bg_processes();
void update_bg_process_list(command_t *cmd, int pid, int status);

#endif
