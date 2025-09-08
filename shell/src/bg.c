#include "../include/lib.h"
#include "../include/globals.h"
#include "../include/bg.h"
#include "../include/command_execution.h"
#include "../include/reveal.h"
#include "../include/hop.h"
#include "../include/log.h"
#include "../include/piping.h"

static bg_process_t bg_processes[MAX_BG_PROCESSES]; // Define MAX_BG_PROCESSES in your header
static int num_bg_processes = 0;
// static int job_counter = 1;

void parse_command(char *cmd, char *argv[])
{
    int i = 0;
    char *token = strtok(cmd, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        argv[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[i] = NULL; // terminate argv[]
}

// ############## LLM Generated Code Begins ##############

void send_to_background(command_t *cmd, char *curr_command, int status)
{
    int input_fd = -1, output_fd = -1;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // Create new process group

        if (setpgid(0, 0) == -1)
        {
            perror("setpgid failed");
        }

        // Redirect stdout and stderr to /dev/null to prevent output to terminal
        // setsid();

        // int devnull = open("/dev/null", O_WRONLY);
        // if (devnull != -1)
        // {
        //     dup2(devnull, STDOUT_FILENO);
        //     dup2(devnull, STDERR_FILENO);
        //     close(devnull);
        // }

        // // Close stdin
        // close(STDIN_FILENO);

        // Execute command
        // char *argv[MAX_ARGS];
        // char cmd_copy[1024];
        // strcpy(cmd_copy, bg_commands);
        // parse_command(cmd_copy, argv);
        // // printf("Sending to background: %s\n", argv[0]);
        // if (execvp(argv[0], argv) == -1)
        // {
        //     // perror("exec failed");
        //     exit(127); // Command not found exit code
        // }

        // Handle input redirection
        if (cmd->input_file == NULL) {
            int devnull_in = open("/dev/null", O_RDONLY);
            if (devnull_in != -1) {
                dup2(devnull_in, STDIN_FILENO);
                close(devnull_in);
            }
        }
        else
        {
            input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1)
            {
                printf("No such file or directory\n");
                exit(1);
            }

            // Redirect stdin to the file
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                perror("dup2 failed");
                close(input_fd);
                exit(1);
            }

            // Close the original file descriptor
            close(input_fd);
        }

        // Handle output redirection
        if (cmd->output_file != NULL)
        {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->append_output)
            {
                flags |= O_APPEND;
            }
            else
            {
                flags |= O_TRUNC;
            }

            output_fd = open(cmd->output_file, flags, 0644);
            if (output_fd == -1)
            {
                perror("open output file failed");
                exit(1);
            }

            // Redirect stdout to the file
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 failed");
                close(output_fd);
                exit(1);
            }

            // Close the original file descriptor
            close(output_fd);
        }
        // Try to execute the command
        // printf("Executing...\n");
        if (execvp(cmd->args[0], cmd->args) == -1)
        {
            perror("exec failed");
            exit(1);
        }
    }
    else
    {
        // parent process

        if (num_bg_processes < MAX_BG_PROCESSES)
        {
            bg_processes[num_bg_processes].pid = pid;
            bg_processes[num_bg_processes].job_num = job_counter;

            // Extract command name (first word)
            char cmd_copy[1024];
            strcpy(cmd_copy, curr_command);
            char *first_word = strtok(cmd_copy, " \t\n");
            if (first_word)
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, curr_command);
                bg_processes[num_bg_processes].cmd_name[sizeof(bg_processes[num_bg_processes].cmd_name) - 1] = '\0';
            }
            else
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, "unknown");
            }

            bg_processes[num_bg_processes].active = 1;
            bg_processes[num_bg_processes].status = 1;

            num_bg_processes++;
        }

        printf("[%d] %d\n", job_counter++, pid);
        fflush(stdout); // Ensure output is printed immediately
    }
}

void execute_custom_fnc_in_bg(command_t *cmd, char *cwd, char *shell_homedir, char *current_command, char *log_file_path, int cmd_count)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // Create new process group

        if (setpgid(0, 0) == -1)
        {
            perror("setpgid failed");
        }

        // Redirect stdout and stderr to /dev/null to prevent output to terminal
        // setsid();
        // int devnull = open("/dev/null", O_WRONLY);
        // if (devnull != -1)
        // {
        //     dup2(devnull, STDOUT_FILENO);
        //     dup2(devnull, STDERR_FILENO);
        //     close(devnull);
        // }

        // // Close stdin
        // close(STDIN_FILENO);
        if (cmd->input_file == NULL) {
            int devnull = open("/dev/null", O_RDONLY);
            if (devnull != -1) {
                dup2(devnull, STDIN_FILENO);
                close(devnull);
            }
        }
        if (strncmp(current_command, "hop", 3) == 0)
        {
            if (cmd->input_file != NULL)
            {
                execute_hop_with_command_t(cmd, cwd, shell_homedir);
            }
            else
            {
                execute_hop(current_command, cwd, shell_homedir);
            }
        }

        else if (strncmp(current_command, "reveal", 6) == 0)
        {
            if (cmd->input_file != NULL)
            {
                // If input file is specified, read the command from the file
                execute_reveal_with_command_t(cmd, cwd, shell_homedir);
            }
            else
            {
                // fwrite(command, sizeof(char), strlen(command), ptr);
                execute_reveal(current_command, cwd, shell_homedir, cmd->output_file);
            }
        }
        else if (strncmp(current_command, "log", 3) == 0)
        {
            FILE *ptr = fopen("logs.txt", "a+");
            if (ptr == NULL)
            {
                perror("Error opening log file");
            }
            else
            {
                execute_log(current_command, ptr, log_file_path, cwd, shell_homedir, cmd_count, sizeof(cwd), cmd->output_file);
            }
            fclose(ptr);
        }
    }
    else
    {
        if (num_bg_processes < MAX_BG_PROCESSES)
        {
            bg_processes[num_bg_processes].pid = pid;
            bg_processes[num_bg_processes].job_num = job_counter;

            // Extract command name (first word)
            char cmd_copy[1024];
            strcpy(cmd_copy, current_command);
            char *first_word = strtok(cmd_copy, " \t\n");
            if (first_word)
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, current_command);
                bg_processes[num_bg_processes].cmd_name[sizeof(bg_processes[num_bg_processes].cmd_name) - 1] = '\0';
            }
            else
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, "unknown");
            }

            bg_processes[num_bg_processes].active = 1;
            bg_processes[num_bg_processes].status = 1; // Running
            num_bg_processes++;
        }

        printf("[%d] %d\n", job_counter++, pid);
        fflush(stdout); // Ensure output is printed immediately
    }
}

void execute_pipeline_in_bg(command_t cmds[], int num_cmds, char *cwd, char *shell_homedir, char *current_command)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // Create new process group

        if (setpgid(0, 0) == -1)
        {
            perror("setpgid failed");
        }

        // Redirect stdout and stderr to /dev/null to prevent output to terminal
        // setsid();
        // int devnull = open("/dev/null", O_WRONLY);
        // if (devnull != -1)
        // {
        //     dup2(devnull, STDOUT_FILENO);
        //     dup2(devnull, STDERR_FILENO);
        //     close(devnull);
        // }

        // // Close stdin
        // close(STDIN_FILENO);
        if (cmds[0].input_file == NULL) {
            int devnull = open("/dev/null", O_RDONLY);
            if (devnull != -1) {
                dup2(devnull, STDIN_FILENO);
                close(devnull);
            }
        }
        execute_pipeline(cmds, num_cmds, cwd, shell_homedir);
    }
    else
    {
        if (num_bg_processes < MAX_BG_PROCESSES)
        {
            bg_processes[num_bg_processes].pid = pid;
            bg_processes[num_bg_processes].job_num = job_counter;

            // Extract command name (first word)
            char cmd_copy[1024];
            strcpy(cmd_copy, current_command);
            char *first_word = strtok(cmd_copy, " \t\n");
            if (first_word)
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, current_command);
                bg_processes[num_bg_processes].cmd_name[sizeof(bg_processes[num_bg_processes].cmd_name) - 1] = '\0';
            }
            else
            {
                strcpy(bg_processes[num_bg_processes].cmd_name, "unknown");
            }

            bg_processes[num_bg_processes].active = 1;
            bg_processes[num_bg_processes].status = 1; // Running
            num_bg_processes++;
        }

        printf("[%d] %d\n", job_counter++, pid);
        fflush(stdout); // Ensure output is printed immediately
    }
}

void check_background_processes()
{
    int status;
    pid_t pid;

    // Check all active background processes
    for (int i = 0; i < num_bg_processes; i++)
    {
        if (bg_processes[i].active)
        {
            // Use WNOHANG to avoid blocking
            pid = waitpid(bg_processes[i].pid, &status, WNOHANG);

            if (pid == bg_processes[i].pid)
            {
                // Process has finished
                // printf("[%d]+ ", bg_processes[i].job_num);

                if (WIFEXITED(status))
                {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status == 0)
                    {
                        printf("%s with pid %d exited normally\n", bg_processes[i].cmd_name, bg_processes[i].pid);
                    }
                    else
                    {
                        // printf("Exit %d", exit_status);
                        printf("%s with pid %d exited abnormally\n", bg_processes[i].cmd_name, bg_processes[i].pid);
                    }
                }
                else if (WIFSIGNALED(status))
                {
                    // Also consider being terminated by a signal as abnormal
                    printf("%s with pid %d exited abnormally\n", bg_processes[i].cmd_name, bg_processes[i].pid);
                }
                // else if (WIFSIGNALED(status))
                // {
                //     int signal_num = WTERMSIG(status);
                //     printf("Terminated by signal %d", signal_num);
                // }
                // else
                // {
                //     printf("Stopped");
                // }

                // printf("\t\t%s (PID: %d)\n", bg_processes[i].cmd_name, bg_processes[i].pid);

                // Mark as inactive
                bg_processes[i].active = 0;
                bg_processes[i].status = 0; // Stopped
            }
            else if (pid == -1 && errno != ECHILD)
            {
                // Error occurred (but not "no child processes")
                perror("waitpid");
            }
            // If pid == 0, the process is still running
        }
    }

    // Clean up inactive processes from the array
    int write_index = 0;
    for (int read_index = 0; read_index < num_bg_processes; read_index++)
    {
        if (bg_processes[read_index].active)
        {
            if (write_index != read_index)
            {
                bg_processes[write_index] = bg_processes[read_index];
            }
            write_index++;
        }
    }
    num_bg_processes = write_index;
}

// Function to list all active background jobs
void list_background_jobs()
{
    if (num_bg_processes == 0)
    {
        return;
    }

    // printf("Active background jobs:\n");
    for (int i = 0; i < num_bg_processes; i++)
    {
        if (bg_processes[i].active)
        {
            printf("[%d]  Running\t\t%s (PID: %d)\n",
                   bg_processes[i].job_num,
                   bg_processes[i].cmd_name,
                   bg_processes[i].pid);
        }
    }
}

bg_process_t *get_job_list()
{
    return bg_processes;
}

int get_num_bg_processes()
{
    return num_bg_processes;
}

// void update_bg_process_list(command_t *cmd, int pid, int status)
void update_bg_process_list(command_t *cmd, int pid, int status)
{
    if (num_bg_processes < MAX_BG_PROCESSES)
    {
        bg_processes[num_bg_processes].pid = pid;
        bg_processes[num_bg_processes].job_num = job_counter;

        // Build command string from command_t structure
        char cmd_str[1024] = "";
        for (int i = 0; cmd->args[i] != NULL; i++)
        {
            strcat(cmd_str, cmd->args[i]);
            strcat(cmd_str, " ");
        }

        // Copy the full command
        strncpy(bg_processes[num_bg_processes].cmd_name, cmd_str,
                sizeof(bg_processes[num_bg_processes].cmd_name) - 1);
        bg_processes[num_bg_processes].cmd_name[sizeof(bg_processes[num_bg_processes].cmd_name) - 1] = '\0';

        bg_processes[num_bg_processes].active = 1;
        bg_processes[num_bg_processes].status = status;

        num_bg_processes++;
        job_counter++;
    }
}

// ############## LLM Generated Code Ends ##############
