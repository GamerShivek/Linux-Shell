#include "../include/log.h"
#include "../include/globals.h"
#include "../include/hop.h"
#include "../include/reveal.h"
#include "../include/command_execution.h"
#include "../include/semi_colon.h"
#include "../include/piping.h"

void init_log_parser(LogParser *p, char *input)
{
    p->input = input;
    p->pos = 0;
    p->length = strlen(input);
}

char log_peek(LogParser *p)
{
    if (p->pos >= p->length)
    {
        return '\0';
    }
    return p->input[p->pos];
}

char log_consume(LogParser *p)
{
    if (p->pos >= p->length)
    {
        return '\0';
    }
    return p->input[p->pos++];
}

void log_skip_whitespace(LogParser *p)
{
    while (p->pos < p->length && isspace(p->input[p->pos]))
    {
        p->pos++;
    }
}

void no_extra_command(char *log_file_path)
{
    FILE *ptr = fopen(log_file_path, "r");
    if (ptr == NULL)
    {
        printf("No logs found.\n");
        return;
    }

    // Read and print all commands from the log file
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), ptr) != NULL)
    {
        // Process the line as needed
        printf("%s", buffer);
    }

    fclose(ptr);
    return;
}
// {
//     rewind(ptr); // Move to the beginning of the file
//     if(ptr == NULL) {
//         printf("No logs found.\n");
//         return;
//     }
//     // while(ptr != NULL)
//     // {
//         char buffer[256];
//         while(fgets(buffer, sizeof(buffer), ptr) != NULL)
//         {
//             // Process the line as needed
//             printf("%s\n", buffer);
//         }
//     // }
//     return;
// }

void purge_logs(char *log_file_path)
{
    if (remove(log_file_path) == 0)
    {
        printf("Logs purged successfully.\n");
    }
    else
    {
        perror("Error purging logs");
    }
}
// {
//     // Close the file first
//     fclose(ptr);

//     // Remove the log file
//     if (remove("logs.txt") == 0) {
//         printf("Logs purged successfully.\n");
//     } else {
//         perror("Error purging logs");
//     }
//     return;
// }

void execute_cmd(char *log_file_path, char *command, int n, char *cwd, char *shell_homedir, int cmd_count, int cwd_size, char *output_file)
{
    FILE *ptr = fopen(log_file_path, "r");
    if (ptr == NULL)
    {
        printf("No logs found.\n");
        return;
    }
    // go to nth line of the file and read the command
    int curr_line = 0;
    char *found_command = NULL;
    char buffer[256];
    printf("n = %d, cmd_cnt = %d\n", n, cmd_count);
    while (fgets(buffer, sizeof(buffer), ptr))
    {
        if (curr_line == cmd_count - n)
        {
            // Found the nth command
            buffer[strcspn(buffer, "\n")] = 0;

            printf("Executing command: %s", buffer);
            found_command = strdup(buffer); // Make a copy
            break;
        }
        curr_line++;
    }
    fclose(ptr);
    if (found_command == NULL)
    {
        printf("Command not found in logs.\n");
        return;
    }
    // if (strncmp(found_command, "hop", 3) == 0)
    // {
    //     // execute_hop(command, cwd);
    //     // fwrite(command, sizeof(char), strlen(command), ptr);
    //     printf("Changing cwd from: %s\n", cwd);
    //     char *new_path = execute_hop(found_command, cwd, shell_homedir);
    //     printf("New path after hop from logs: %s\n", new_path);
    //     printf("Hop command executed successfully.\n");
    //     if (new_path != NULL)
    //     {
    //         // Copy the new path back to cwd
    //         strncpy(cwd, new_path, cwd_size - 1);
    //         cwd[cwd_size - 1] = '\0';
    //         printf("%d\n", cwd_size);
    //         printf("Current working directory updated to (this is inside log): %s\n", cwd);
    //     }
    // }
    // if (strncmp(found_command, "reveal", 6) == 0)
    // {
    //     // fwrite(command, sizeof(char), strlen(command), ptr);
    //     execute_reveal(found_command, cwd, shell_homedir, output_file);
    // }
    // char semi_colon_separated_commands[64][1024];
    Command semi_colon_separated_commands[64];
    command_t cmds[MAX_CMDS]; // MAX_CMDS = 16
    int num_cmds = 0;
    int num_semi_colon_commands = separate_commands_by_semicolon(found_command, semi_colon_separated_commands, 64);
    for (int i = 0; i < num_semi_colon_commands; i++)
    {
        // strcpy(command, semi_colon_separated_commands[i]);
        // Only trim leading and trailing whitespace, preserve internal spaces
        char *trimmed_cmd = semi_colon_separated_commands[i].cmd;

        // Skip leading whitespace
        while (*trimmed_cmd == ' ' || *trimmed_cmd == '\t')
        {
            trimmed_cmd++;
        }

        // Remove trailing whitespace (but preserve internal spaces for pipes, etc.)
        int len = strlen(trimmed_cmd);
        while (len > 0 && (trimmed_cmd[len - 1] == ' ' || trimmed_cmd[len - 1] == '\t'))
        {
            len--;
        }

        // Create a properly null-terminated string
        char current_command[1024];
        strncpy(current_command, trimmed_cmd, len);
        current_command[len] = '\0';

        if (strlen(current_command) == 0)
        {
            continue; // Skip empty commands
        }

        // Create copies for parsing
        char *input_copy = strdup(current_command);
        char *input_for_command_name = strdup(current_command);

        printf("Command %d: '%s'\n", i + 1, current_command); // Added quotes for debugging
        if (parse_pipeline(input_copy, cmds, &num_cmds) < 0)
        {
            printf("Error parsing pipeline\n");
            free(input_copy);
            free(input_for_command_name);
            continue;
        }

        else
        {
            if (num_cmds == 1)
            {
                command_t cmd = cmds[0];
                if (strncmp(current_command, "hop", 3) == 0)
                {
                    // execute_hop(command, cwd);

                    char *new_path = NULL;
                    if (cmd.input_file != NULL)
                    {
                        // If input file is specified, read the command from the file
                        new_path = execute_hop_with_command_t(&cmd, cwd, shell_homedir);
                    }
                    else
                    {
                        new_path = execute_hop(current_command, cwd, shell_homedir);
                    }
                    printf("Hop command executed successfully.\n");
                    if (new_path != NULL)
                    {
                        // Copy the new path back to cwd
                        strncpy(cwd, new_path, sizeof(cwd) - 1);
                        cwd[sizeof(cwd) - 1] = '\0';
                        printf("Current working directory updated to: %s\n", cwd);
                    }
                }
                else if (strncmp(current_command, "reveal", 6) == 0)
                {
                    if (cmd.input_file != NULL)
                    {
                        // If input file is specified, read the command from the file
                        execute_reveal_with_command_t(&cmd, cwd, shell_homedir);
                    }
                    else
                    {
                        // fwrite(command, sizeof(char), strlen(command), ptr);
                        execute_reveal(current_command, cwd, shell_homedir, cmd.output_file);
                    }
                }
                else
                {
                    execute_command_bash(&cmd);
                    free(input_copy);
                    free(input_for_command_name);
                }
            }
            else
            {
                // Execute pipeline
                printf("Executing pipeline with %d commands\n", num_cmds);
                execute_pipeline(cmds, num_cmds, cwd, shell_homedir);
            }
        }
    }
    free(found_command); // Free the allocated memory for the command
    return;
}

void check_command(char *command, LogParser *p, char *log_file_path, char *cwd, char *shell_homedir, int cmd_count, int cwd_size, char *output_file)
{
    // Check if the command is of the form log purge or log execute <n>
    if (strncmp(command, "log purge", 9) == 0)
    {
        // Handle log purge command
        printf("Purging logs...\n");
        purge_logs(log_file_path);
        return;
    }

    else if (strncmp(command, "log execute", 11) == 0)
    {
        // Handle log execute command
        char *n_str = command + 12; // Skip "log execute "
        int n = atoi(n_str);
        if (n > 0)
        {
            printf("Executing last %d command from logs...\n", n);
            // Logic to execute last n commands
            execute_cmd(log_file_path, command, n, cwd, shell_homedir, cmd_count, cwd_size, output_file);
        }
    }
    else if (strcmp(command, "log") == 0)
    {
        // Handle log command without arguments
        no_extra_command(log_file_path);
    }
    else
    {
        // Invalid log command
        printf("Invalid log command!\n");
    }
    return;
}

void execute_log(char *command, FILE *ptr, char *log_file_path, char *cwd, char *shell_homedir, int cmd_count, int cwd_size, char *output_file)
{
    // int cmd_count = 0;
    LogParser l;
    init_log_parser(&l, command);
    check_command(command, &l, log_file_path, cwd, shell_homedir, cmd_count, cwd_size, output_file);
    return;
}

bool command_exists(char **command_list, char *command, int cmd_count)
{
    for (int i = 0; i < cmd_count; i++)
    {
        if (strcmp(command_list[i], command) == 0)
        {
            return true; // Command already exists
        }
    }
    return false;
}

bool command_exists_in_buffer(char command_buffer[15][512], char *command, int cmd_count)
{
    int commands_to_check = (cmd_count < 15) ? cmd_count : 15;

    for (int i = 0; i < commands_to_check; i++)
    {
        if (strcmp(command_buffer[i], command) == 0)
        {
            return true; // Command already exists
        }
    }
    return false;
}
// ############## LLM Generated Code Begins ##############

void add_command_to_log(char *command, char *log_file_path, char command_buffer[15][512], int *total_cmd_count, int *cmd_count)
{
    // Add to circular buffer
    int buffer_index = (*total_cmd_count) % 15;
    strcpy(command_buffer[buffer_index], command);
    (*cmd_count)++;
    if (*cmd_count > 15)
    {
        *cmd_count = 15; // Limit to last 15 commands
    }
    (*total_cmd_count)++;

    // Rewrite entire log file with current buffer contents
    FILE *ptr = fopen(log_file_path, "w"); // Open in write mode to overwrite
    if (ptr != NULL)
    {
        int start_index, commands_to_write;

        if (*total_cmd_count <= 15)
        {
            // Haven't wrapped around yet, write from beginning
            start_index = 0;
            commands_to_write = *total_cmd_count;
        }
        else
        {
            // Have wrapped around, write the last 15 commands in order
            start_index = (*total_cmd_count) % 15;
            commands_to_write = 15;
            // (*cmd_count) = 15;
        }

        // Write commands in the correct order (oldest to newest)
        for (int i = 0; i < commands_to_write; i++)
        {
            int index = (start_index + i) % 15;
            fprintf(ptr, "%s\n", command_buffer[index]);
        }

        fclose(ptr);
    }
}

// ############## LLM Generated Code Ends ##############