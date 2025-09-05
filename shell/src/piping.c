#include "../include/lib.h"
#include "../include/command_execution.h"
#include "../include/reveal.h"
#include "../include/bg.h"
//  ############## LLM Generated Code Begins ##############

int parse_pipeline(char *input, command_t cmds[], int *num_cmds)
{
    char *saveptr;
    char *segment = strtok_r(input, "|", &saveptr);
    int count = 0;

    while (segment != NULL && count < MAX_CMDS)
    {
        // Trim leading spaces
        while (*segment == ' ' || *segment == '\t')
            segment++;

        // Parse this segment into a command
        parse_command_with_redirection(segment, &cmds[count]);
        count++;
        // printf("Segment %d: %s\n", count, segment);
        segment = strtok_r(NULL, "|", &saveptr);
    }

    *num_cmds = count;
    return count;
}

void setup_redirection(command_t *cmd)
{
    if (cmd->input_file)
    {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0)
        {
            perror("open input");
            _exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (cmd->output_file)
    {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0)
        {
            perror("open output");
            _exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void execute_pipeline(command_t cmds[], int num_cmds, char *cwd, char *shell_homedir)
{
    int pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];

    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdin < 0 || saved_stdout < 0)
    {
        perror("dup failed");
        return;
    }

    // Create pipes
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            if (i == 0) {
                // NEW: First child creates the process group
                setpgid(0, 0);
            } else {
                // NEW: Other children join the group
                setpgid(0, fg_pid); // fg_pgid is set by parent before this fork
            }
            // Child process
            if (i > 0 && !cmds[i].input_file)
            { // Not first command → get input from previous pipe
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < num_cmds - 1 && !cmds[i].output_file)
            { // Not last command → output to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Apply input/output redirection
            setup_redirection(&cmds[i]);

            // Close all pipe fds in child
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // printf("Testing...\n");
            // printf("Executing command: %s\n", cmds[i].args[0]);
            char full_cmd[1024] = {0};
            for (int j = 0; j < cmds[i].argc; j++)
            {
                strcat(full_cmd, cmds[i].args[j]);
                if (j < cmds[i].argc - 1)
                strcat(full_cmd, " ");
            }
            
            // Now call reveal with full command
            if (strcmp(cmds[i].args[0], "reveal") == 0)
            {
                execute_reveal(full_cmd, cwd, shell_homedir, cmds[i].output_file);
                fflush(stdout); // flush for pipes
                _exit(0);
            }
            // Exec command
            else
            {
                execvp(cmds[i].args[0], cmds[i].args);
                // perror("execvp");
                fprintf(stderr, "Command not found!\n");
                _exit(1);
            }
        }
        else if (pids[i] < 0)
        {
            perror("fork");
            // Close all pipes before exiting
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return;
        }
        else
        {
            // Parent process
            if (i == 0)
            {
                fg_pid = pids[i]; 
            }
        }
    }

    // Parent closes pipes
    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    tcsetpgrp(STDIN_FILENO, fg_pid);
    bool job_added_to_bg = false;

    // Wait for all children
    for (int i = 0; i < num_cmds; i++)
    {
        int status;
        if(waitpid(pids[i], &status, WUNTRACED) > 0)
        {
            if(WIFSTOPPED(status) && !job_added_to_bg)
            {
                // execute_pipeline_in_bg(cmds, num_cmds, cwd, shell_homedir, );
                // If one process stops, the whole job is stopped.
                // Create a representative command string for the job.
                char pipeline_cmd_str[1024] = ""; 
                for(int j = 0; j < num_cmds; j++) {
                    strcat(pipeline_cmd_str, cmds[j].args[0]);
                    if (j < num_cmds - 1) strcat(pipeline_cmd_str, " | ");
                }
                
                printf("\n[%d] Stopped %s\n", job_counter, pipeline_cmd_str);
                
                // Add the entire pipeline as a single background job
                // Note: You may need a new function in bg.c for this
                // For now, we'll just add the first command as a placeholder
                command_t pipeline_job_cmd;
                pipeline_job_cmd.args[0] = pipeline_cmd_str;
                pipeline_job_cmd.args[1] = NULL;
                
                update_bg_process_list(&pipeline_job_cmd, fg_pid, 0); 
                job_added_to_bg = true;
            }
        }
    }
    tcsetpgrp(STDIN_FILENO, getpgrp());
    fg_pid = 0;

    // restore stdin and stdout of the parent shell
    if (dup2(saved_stdin, STDIN_FILENO) == -1)
    {
        perror("restoring stdin failed");
    }
    if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    {
        perror("restoring stdout failed");
    }
    close(saved_stdin);
    close(saved_stdout);
}

// ############## LLM Generated Code Ends ##############