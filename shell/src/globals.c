#include "globals.h"

char prev_dir[1024] = {0};  // Global variable to store previous directory
struct passwd *pw = NULL;
volatile pid_t fg_pid = 0;
int job_counter = 1;
