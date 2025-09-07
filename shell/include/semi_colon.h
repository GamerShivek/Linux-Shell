#ifndef SEMI_COLON_H
#define SEMI_COLON_H

#include "lib.h"

typedef struct {
    char cmd[1024];  // command string
    int bg;          // 1 = run in background, 0 = run in foreground
    int is_custom;
} Command;

int separate_commands_by_semicolon(const char *input, Command commands[], int max_commands);
// int separate_commands_by_ampersand(const char *input, char bg_commands[][1024], int max_commands);
#endif