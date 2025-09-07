#include "../include/lib.h"
#include "../include/parser.h"
#include "../include/semi_colon.h"

// Store the commands separated by semicolons

int separate_commands_by_semicolon(const char *input, Command commands[], int max_commands) {
    int count = 0;
    const char *start = input;
    const char *ptr = input;

    while (*ptr != '\0' && count < max_commands) {
        if (*ptr == ';' || *ptr == '&') {
            size_t len = ptr - start;
            if (len > 0 && len < sizeof(commands[count].cmd)) {
                strncpy(commands[count].cmd, start, len);
                commands[count].cmd[len] = '\0';
                commands[count].bg = (*ptr == '&') ? 1 : 0; // background if &

                if(strncmp(commands[count].cmd, "reveal", 6) == 0 ||
                strncmp(commands[count].cmd, "hop", 3) == 0 ||
                strncmp(commands[count].cmd, "log", 3) == 0) {
                    commands[count].is_custom = 1;
                } else {
                    commands[count].is_custom = 0;
                }
                count++;
            }
            start = ptr + 1; // move past delimiter
        }
        ptr++;
    }

    // Handle last command if any
    if (ptr != start && count < max_commands) {
        size_t len = ptr - start;
        if (len > 0 && len < sizeof(commands[count].cmd)) {
            strncpy(commands[count].cmd, start, len);
            commands[count].cmd[len] = '\0';
            commands[count].bg = 0; // default: foreground (no & at end)
            if(strncmp(commands[count].cmd, "reveal", 6) == 0 ||
                strncmp(commands[count].cmd, "hop", 3) == 0 ||
                strncmp(commands[count].cmd, "log", 3) == 0) {
                    commands[count].is_custom = 1;
                } else {
                    commands[count].is_custom = 0;
                }
            count++;
        }
    }

    return count; // number of parsed commands
}

// int separate_commands_by_ampersand(const char *input, char bg_commands[][1024], int max_commands) {
//     int count = 0;
//     const char *start = input;
//     const char *ptr = input;
    
//     while (*ptr != '\0' && count < max_commands) {
//         if (*ptr == '&') {
//             size_t len = ptr - start;
//             if (len > 0 && len < 1024) {
//                 strncpy(bg_commands[count], start, len);
//                 bg_commands[count][len] = '\0'; // Null-terminate the string
//                 count++;
//             }
//             start = ptr + 1; // Move past the ampersand
//         }
//         ptr++;
//     }
//     return count; // Return number of commands found
// }
