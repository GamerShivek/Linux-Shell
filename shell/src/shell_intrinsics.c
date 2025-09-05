#include "../include/lib.h"
#include "../include/globals.h"
#include "../include/bg.h"
// #include "bg.c"

void execute_ping(char *input)
{
    int space_idx = -1;
    int len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        if (input[i] == ' ')
        {
            space_idx = i;
            break;
        }
    }

    int pid = atoi(&input[space_idx + 1]);
    for (int i = space_idx + 1; i < len; i++)
    {
        if (input[i] == ' ')
        {
            space_idx = i;
            break;
        }
    }
    int sig_num = 0;
    sig_num = atoi(&input[space_idx + 1]);
    sig_num = sig_num % 32;

    int num_bg_processes = get_num_bg_processes();
    bg_process_t *bg_processes = get_job_list();
    for (int i = 0; i < num_bg_processes; i++)
    {
        if (bg_processes[i].pid == pid && bg_processes[i].active)
        {
            if (kill(bg_processes[i].pid, sig_num) == 0) {
                printf("Sent signal %d to process with pid %d\n", sig_num, pid);
                return;
            } else {
                perror("ping: kill failed");
            }
        }
    }
    printf("No such process found\n");
    return;
}

void handle_sigint(int sig)
{
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGINT); // Send SIGINT to the foreground process group
    }
    signal(SIGINT, handle_sigint);
}

void handle_sigkill()
{
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGKILL); // Send SIGKILL to the foreground process group
    }
    printf("logout\n");
    exit(0);
}

void handle_sigstp(int sig)
{
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGTSTP); // Send SIGTSTP to the foreground process group
        // signal(SIGTSTP, handle_sigstp);
    }
    signal(SIGTSTP, handle_sigstp);
}

void list_activities()
{
    int num_bg_processes = get_num_bg_processes();
    if (num_bg_processes == 0)
    {
        printf("No background processes\n");
        return;
    }

    bg_process_t *bg_processes = get_job_list();

    bg_process_t *sorted_processes = (bg_process_t *)malloc(num_bg_processes * sizeof(bg_process_t));

    memcpy(sorted_processes, bg_processes, num_bg_processes * sizeof(bg_process_t));

    for (int i = 0; i < num_bg_processes - 1; i++)
    {
        for (int j = 0; j < num_bg_processes - i - 1; j++)
        {
            if (strcmp(sorted_processes[j].cmd_name, sorted_processes[j + 1].cmd_name) > 0)
            {
                bg_process_t temp = sorted_processes[j];
                sorted_processes[j] = sorted_processes[j + 1];
                sorted_processes[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < num_bg_processes; i++)
    {
        if (sorted_processes[i].active)
        {
            printf("[%d] : %s - %s\n",
                   sorted_processes[i].pid,
                   sorted_processes[i].cmd_name,
                   sorted_processes[i].status == 1 ? "Running" : "Stopped");
        }
    }
}

void bring_to_fg(int job_number, bool num_provided)
{
    int num_bg_processes = get_num_bg_processes();
    bg_process_t *bg_processes = get_job_list();
    bg_process_t *target_job = NULL;
    int idx = -1;

    if (num_provided)
    {
        for (int i = 0; i < num_bg_processes; i++)
        {
            if (bg_processes[i].job_num == job_number && bg_processes[i].active)
            {
                target_job = &bg_processes[i];
                idx = i;
                bg_processes[i].active = 0; // Mark as inactive
                break;
            }
        }
    }
    else
    { // Find the most recent job
        for (int i = num_bg_processes - 1; i >= 0; i--)
        {
            if (bg_processes[i].active)
            {
                target_job = &bg_processes[i];
                bg_processes[i].active = 0; // Mark as inactive
                idx = i;
                break;
            }
        }
    }

    if (target_job == NULL)
    {
        fprintf(stderr, "No such job\n");
        return;
    }
    pid_t pid = target_job->pid;
    char cmd_name[1024];
    strcpy(cmd_name, target_job->cmd_name);
    printf("%s\n", cmd_name);

    tcsetpgrp(STDIN_FILENO, pid);
    if (kill(-pid, SIGCONT) < 0)
    {
        perror("kill (SIGCONT) failed");
        tcsetpgrp(STDIN_FILENO, getpgrp());
        return;
    }

    fg_pid = pid;
    int status;
    if (waitpid(pid, &status, WUNTRACED) > 0)
    {
        if (WIFSTOPPED(status))
        {
            printf("\n[%d] Stopped %s\n", bg_processes[idx].job_num, cmd_name);
            
            // command_t temp_cmd; // Create a temporary cmd_t for update_bg_process_list
            // temp_cmd.args[0] = cmd_name;
            // temp_cmd.args[1] = NULL;
            // update_bg_process_list(&temp_cmd, pid, 0);
            bg_processes[idx].active = 1; // Mark as active again
            bg_processes[idx].status = 0; // Mark as stopped
        }
    }
    else
    {
        // Process terminated
        bg_processes[idx].active = 0; // Mark as inactive
    }
    tcsetpgrp(STDIN_FILENO, getpgrp());
    fg_pid = 0;
}

void run_stopped_bg_command(int job_number)
{
    int num_bg_processes = get_num_bg_processes();
    bg_process_t *bg_processes = get_job_list();
    bg_process_t *target_job = NULL;

    for (int i = 0; i < num_bg_processes; i++)
    {
        if (bg_processes[i].job_num == job_number)
        {
            if (bg_processes[i].active && bg_processes[i].status == 0)
            {
                target_job = &bg_processes[i];
                bg_processes[i].status = 1; // Mark as running
                break;
            }
            else if(bg_processes[i].active && bg_processes[i].status == 1) {
                printf("Job already running\n");
                return;
            }
        }
    }

    if (target_job == NULL)
    {
        fprintf(stderr, "No such job\n");
        return;
    }

    pid_t pid = target_job->pid;
    char cmd_name[1024];
    strcpy(cmd_name, target_job->cmd_name);
    printf("[%d] %s &\n", target_job->job_num, cmd_name);

    if (kill(-pid, SIGCONT) < 0)
    {
        perror("kill (SIGCONT) failed");
        return;
    }

    // Update the job status to running
    // command_t temp_cmd; // Create a temporary cmd_t for update_bg_process_list
    // temp_cmd.args[0] = cmd_name;
    // temp_cmd.args[1] = NULL;
    // update_bg_process_list(&temp_cmd, pid, 1);
}