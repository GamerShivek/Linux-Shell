#ifndef GLOBALS_H
#define GLOBALS_H

#include "lib.h"

extern char prev_dir[1024];  // Global variable to store previous directory
extern struct passwd *pw;
extern volatile pid_t fg_pid;
extern int job_counter;

#define MAX_ARGS 64
#define MAX_CMDS 16

#endif