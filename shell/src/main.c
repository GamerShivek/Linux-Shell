#include "../include/lib.h"
#include "../include/parser.h"
#include "../include/hop.h"
#include "../include/reveal.h"
#include "../include/globals.h"
#include "../include/log.h"
#include "../include/command_execution.h"
#include "../include/piping.h"
#include "../include/semi_colon.h"
#include "../include/bg.h"
#include "../include/shell_intrinsics.h"

int cmds_in_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return 0; // File doesn't exist or can't be opened
    }

    int count = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file))
    {
        count++;
    }

    fclose(file);
    return count;
}

void load_log_file(char *log_file_path, char command_list[15][512], int *total_cmd_count, int *cmd_count)
{
    FILE *file = fopen(log_file_path, "r");
    if (file)
    {
        char line[512];
        int count = 0;

        while (fgets(line, sizeof(line), file))
        {
            // Remove newline
            line[strcspn(line, "\n")] = 0;

            strcpy(command_list[count % 15], line);
            count++;
        }

        fclose(file);

        *total_cmd_count = count;
        *cmd_count = (count < 15) ? count : 15;
    }
    return;
}

bool is_whitespace_only(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}
// ----------------------------------------------------------------------------
int main()
{

    signal(SIGTTOU, SIG_IGN); // Tell the shell to ignore the "stop writing" signal
    char cwd[1024];
    char *homedir;

    // LLM generated code begins
    char hostname[256];
    // struct passwd *pw;

    // Get username using getpwuid
    pw = getpwuid(getuid()); // getuid() returns current user ID
    if (!pw)
    {
        perror("getpwuid");
        return 1;
    }
    // Get hostname
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        perror("gethostname");
        return 1;
    }

    // printf("Username: %s\n", pw->pw_name);
    // printf("System Name: %s\n", hostname);
    // LLM generated code ends

    homedir = pw->pw_dir;
    if (!getcwd(cwd, sizeof(cwd)))
    {
        perror("Couldn't get cwd path\n");
        exit(1);
    }
    // char* prev_dir = cwd;
    // char prev_dir[1024] = {0};
    char shell_homedir[1024] = {0};
    strcpy(shell_homedir, cwd);
    char log_file_path[2048];
    snprintf(log_file_path, sizeof(log_file_path), "%s/logs.txt", shell_homedir);

    int cmd_count = cmds_in_file(log_file_path);
    int total_cmd_count = cmds_in_file(log_file_path);

    static char command_list[15][512];
    for (int i = 0; i < 15; i++)
    {
        command_list[i][0] = '\0';
    }
    // command_t cmd;
    // char semi_colon_separated_commands[64][1024];
    // char bg_commands[64][1024];

    Command bg_semi_commands[64];

    command_t cmds[MAX_CMDS]; // MAX_CMDS = 16
    int num_cmds = 0;

    load_log_file(log_file_path, command_list, &total_cmd_count, &cmd_count);

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigstp);
    // strncpy(prev_dir, cwd, sizeof(prev_dir)-1);
    while (1)
    {
        // ------------------------------------------------------------------
        char *curr_dir = cwd;
        // char **command_list = (char **)malloc(sizeof(char *) * 15);

        // for (int i = 0; i < 15; i++)
        // {
        //     command_list[i] = (char *)malloc(sizeof(char) * 512);
        // }
        // int cmd_count = 0;

        // ------------------------------------------------------------------

        if (strncmp(cwd, homedir, strlen(homedir)) == 0)
        {
            int rel_path_len = strlen(curr_dir) - strlen(shell_homedir);
            if (rel_path_len == 0)
            {
                printf("<%s@%s:~", pw->pw_name, hostname);
            }
            else if (rel_path_len > 0)
            {
                printf("<%s@%s:~%s", pw->pw_name, hostname, curr_dir + strlen(shell_homedir));
            }
            else if (rel_path_len < 0)
            {
                printf("<%s@%s:%s", pw->pw_name, hostname, cwd);
            }
            printf("> ");
            fflush(stdout);
        }
        else
        {
            printf("<%s@%s:%s> ", pw->pw_name, hostname, cwd);
        }
        // -----------------------------------------------------------------------

        // char *args[MAX_ARGS];
        // int num_args;

        char input[1024];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            if(feof(stdin)) {
                handle_sigkill();
            }
            else if (ferror(stdin)) {
                // Case 2: A stream error occurred.
                // Check if it was specifically a signal interruption.
                if (errno == EINTR) {
                    // It was an interruption. Clear the error and continue the loop
                    // to wait for new input. This is safe.
                    printf("\n");
                    clearerr(stdin);
                    continue;
                } else {
                    // It was a different, likely unrecoverable error (like the one
                    // you found). The safest action is to exit the shell.
                    handle_sigkill();
                }
            }
        }
        check_background_processes();
        if (is_whitespace_only(input)) {
            continue; // Ignore and show a new prompt
        }

        if(strncmp(input, "ping", 4) == 0)
        {
            execute_ping(input);
            continue;
        }

        if(strcmp(input, "\n") == 0) {
            continue; // Ignore empty input
        }

        // Remove newline
        input[strcspn(input, "\n")] = 0;

        
        char *command = extract_command(input);
        
        
        // if (num_args == 0) {
        //     free(input_copy);
        //     continue;
        // }
        char* test_env = getenv("SHELL_TEST_ENV");
        if (test_env == NULL)
        {
            if (strstr(command, "log") == NULL)
            {
                if (!command_exists_in_buffer(command_list, command, cmd_count))
                {
                    add_command_to_log(command, log_file_path, command_list, &total_cmd_count, &cmd_count);
                }
            }
        }
        
        if (!is_valid_command(command))
        {
            printf("Invalid Syntax!\n");
            continue;
        }
        
        if(strcmp(command, "activities") == 0)
        {
            list_activities();
            continue;
        }

        if(strcmp(command, "fg") == 0)
        {
            bool num_provided = false;
            int job_number = -1;
            bring_to_fg(job_number, num_provided);
            continue;
        }
        else if(strncmp(command, "fg ", 3) == 0)
        {
            bool num_provided = true;
            int job_number = atoi(command + 3);

            bring_to_fg(job_number, num_provided);
            continue;
        }
        else if(strncmp(command, "bg ", 3) == 0)
        {
            int job_number = atoi(command + 3);
            run_stopped_bg_command(job_number);
            continue;
        }
        // char* command_name = extract_command_name(input_for_command_name);

        // if (command_name == NULL) {
        //     free(input_copy);
        //     free(input_for_command_name);
        //     continue;
        // }

        // Parse the full input into arguments
        // if (parse_command_with_redirection(input_copy, &cmd) == 0)
        // {
        //     free(input_copy);
        //     free(input_for_command_name);
        //     continue;
        // }

        int num_semi_colon_commands = separate_commands_by_semicolon(command, bg_semi_commands, 64); // Also separeates them by &
        // int num_bg_commands = separate_commands_by_ampersand(command, bg_commands, 64);
        
        // debug step:
        // print all commands separated by semicolon
        // for (int i = 0; i < num_semi_colon_commands; i++) {
        //     printf("Command %d: '%s' (bg=%d)\n", i + 1, bg_semi_commands[i].cmd, bg_semi_commands[i].bg);
        // }


        // send_to_background(bg_commands, num_bg_commands);

        for (int i = 0; i < num_semi_colon_commands; i++)
        {
            // strcpy(command, semi_colon_separated_commands[i]);
            // Only trim leading and trailing whitespace, preserve internal spaces
            char *trimmed_cmd = bg_semi_commands[i].cmd;

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

            if (bg_semi_commands[i].bg)
            {
                // Command should run in background
                if (parse_pipeline(input_copy, cmds, &num_cmds) < 0)
                {
                    printf("Error parsing pipeline\n");
                    free(input_copy);
                    free(input_for_command_name);
                    continue;
                }
                if (num_cmds == 1)
                {
                    command_t cmd = cmds[0];
                    if (strncmp(current_command, "hop", 3) == 0 ||
                        strncmp(current_command, "reveal", 6) == 0 ||
                        strncmp(current_command, "log", 3) == 0)
                    {
                        // printf("Executing custom command in background: %s\n", current_command);
                        execute_custom_fnc_in_bg(&cmd, cwd, shell_homedir, current_command, log_file_path, cmd_count);
                    }
                    else
                    {
                        // send_to_background(cmd.input_file ? cmd.input_file : current_command);
                        send_to_background(&cmd, current_command, 1);
                    }
                }
                else {
                    // printf("Executing pipeline in background with %d commands\n", num_cmds);
                    execute_pipeline_in_bg(cmds, num_cmds, cwd, shell_homedir, current_command);
                }
                
                // send_to_background(bg_semi_commands[i].cmd);
                free(input_copy);
                free(input_for_command_name);
                continue; // Move to next command
            }
            else
            {
                // printf("Command %d: '%s'\n", i + 1, current_command); // Added quotes for debugging
                if (parse_pipeline(input_copy, cmds, &num_cmds) < 0)
                {
                    printf("Error parsing pipeline\n");
                    free(input_copy);
                    free(input_for_command_name);
                    continue;
                }

                else
                {
                    // printf("Running command: %s\n", current_command);
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
                            // printf("Hop command executed successfully.\n");
                            if (new_path != NULL)
                            {
                                // Copy the new path back to cwd
                                strncpy(cwd, new_path, sizeof(cwd) - 1);
                                cwd[sizeof(cwd) - 1] = '\0';
                                // printf("Current working directory updated to: %s\n", cwd);
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

                        else if (strncmp(current_command, "log", 3) == 0)
                        {
                            // Log command execution
                            FILE *ptr = fopen("logs.txt", "a+");
                            if (ptr == NULL)
                            {
                                perror("Error opening log file");
                                continue;
                            }
                            else
                            {
                                if (strcmp(current_command, "log purge") == 0)
                                {
                                    total_cmd_count = 0;
                                    cmd_count = 0;
                                    for (int i = 0; i < 15; i++)
                                    {
                                        command_list[i][0] = '\0';
                                    }
                                }
                                // printf("CWD: %s\n", cwd);
                                execute_log(current_command, ptr, log_file_path, cwd, shell_homedir, cmd_count, sizeof(cwd), cmd.output_file);
                            }
                            fclose(ptr);
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
                        // printf("Executing pipeline with %d commands\n", num_cmds);
                        execute_pipeline(cmds, num_cmds, cwd, shell_homedir);
                    }
                }
            }
        }
        free(command);
        if (!getcwd(cwd, sizeof(cwd)))
        {
            perror("Couldn't get cwd path\n");
            exit(1);
        }
    }
}

// Change the home-dir logic from calculating the real path to just using the home directory
// remove faltu ke printfs!!
// Understand bg.c code
// piped command sent to bg not giving exit status..