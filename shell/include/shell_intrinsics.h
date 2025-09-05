#ifndef SHELL_INTRINSICS_H
#define SHELL_INTRINSICS_H

#include "lib.h"

void execute_ping(char* input);
void handle_sigint(int sig);
void handle_sigkill();
void handle_sigstp(int sig);
void list_activities();
void bring_to_fg(int job_number, bool num_provided);
void run_stopped_bg_command(int job_number);

#endif