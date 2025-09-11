#include "../include/command_execution.h"
#include "../include/globals.h"
#include "../include/bg.h"
// ############## LLM Generated Code Begins ##############

// Function to parse input into tokens
// int parse_input_for_args(char *input, char **args) {
//     char *token;
//     int i = 0;
    
//     // Tokenize the input
//     token = strtok(input, " \t");
//     while (token != NULL && i < MAX_ARGS - 1) {
//         args[i] = token;
//         i++;
//         token = strtok(NULL, " \t");
//     }
//     args[i] = NULL;  // Null terminate the array
    
//     return i;  // Return number of arguments
// }

// // Function to execute command using exec
// void execute_command_bash(char **args) {
//     pid_t pid;
    
//     // Create a child process
//     pid = fork();
    
//     if (pid == 0) {
//         // Child process
//         // Try to execute the command
//         if (execvp(args[0], args) == -1) {
//             // perror("exec failed");
//             exit(1);
//         }
//     } else if (pid < 0) {
//         // Fork failed
//         perror("fork failed");
//     } else {
//         // Parent process - wait for child to complete
//         wait(NULL);
//     }
// }


// Function to extract command name from input
char* extract_command_name(char *input) {
    char *token = strtok(input, " \t");
    return token;
}

// Function to parse input with redirection support
int parse_command_with_redirection(char *input, command_t *cmd) {
    char *token;
    int i = 0;
    
    // Initialize command structure
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = false;
    int input_fd = -1, output_fd = -1;
    
    // Tokenize the input
    token = strtok(input, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            // Input redirection - get the filename
            token = strtok(NULL, " \t");
            if (token != NULL) {
                // If multiple input redirections, only the last one takes effect
                cmd->input_file = token;
                input_fd = open(cmd->input_file, O_RDONLY);
                if (input_fd == -1) {
                    fprintf(stderr, "No such file or directory\n");
                    // printf("No such file or directory\n");
                    return -1;
                }
            }
        } else if (strcmp(token, ">") == 0) {
            // Output redirection - get the filename
            token = strtok(NULL, " \t");
            if (token != NULL) {
                cmd->output_file = token;
                int flags = O_WRONLY | O_CREAT;
                if (cmd->append_output) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                output_fd = open(cmd->output_file, flags, 0644);
                if (output_fd == -1) {
                    fprintf(stderr, "Unable to create file for writing\n");
                    return -1;
                }
                cmd->append_output = false;
            }
        } else if (strcmp(token, ">>") == 0) {
            // Output redirection (append) - get the filename
            token = strtok(NULL, " \t");
            if (token != NULL) {
                cmd->output_file = token;
                cmd->append_output = true;
                int flags = O_WRONLY | O_CREAT;
                if (cmd->append_output) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                output_fd = open(cmd->output_file, flags, 0644);
                if (output_fd == -1) {
                    fprintf(stderr, "Unable to create file for writing\n");
                    return -1;
                }
            }
        } else {
            // Regular argument
            cmd->args[i] = token;
            i++;
        }
        token = strtok(NULL, " \t");
    }
    
    cmd->args[i] = NULL;  // Null terminate the array
    cmd->argc = i;
    
    return i;  // Return number of arguments
}

// Function to execute command with redirection support
void execute_command_bash(command_t *cmd) {
    if (strcmp(cmd->args[0], "cd") == 0) {
        // Handle 'cd' command
        char target_dir[1024];
        if (cmd->argc < 2) {
            // No directory provided, default to home
            char *home_dir = getenv("HOME");
            if (home_dir == NULL) {
                home_dir = getpwuid(getuid())->pw_dir;
            }
            strcpy(target_dir, home_dir);
        } else {
            // Use the provided directory path
            strcpy(target_dir, cmd->args[1]);
        }

        if (chdir(target_dir) != 0) {
            perror("shell");
        }
        // Note: The directory is now changed. For the prompt to update,
        // your main loop needs to call getcwd() to refresh its state.
        return; // Done with built-in, no need to fork.
    }
    if (strcmp(cmd->args[0], "exit") == 0) {
        // Handle 'exit' command
        exit(0);
    }
    if (strcmp(cmd->args[0], "pwd") == 0) {
        // Handle 'pwd' command
        char current_dir[1024];
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            printf("%s\n", current_dir);
        } else {
            perror("pwd");
        }
        return; // Done with built-in.
    }


    // get complete command string for job listing
    char full_cmd[1024] = {0};
    for (int i = 0; i < cmd->argc; i++) {
        strcat(full_cmd, cmd->args[i]);
        if (i < cmd->argc - 1) strcat(full_cmd, " ");
    }

    pid_t pid;
    int input_fd = -1, output_fd = -1;
    
    // Create a child process
    pid = fork();
    
    if (pid == 0) {
        // Child process
        setpgid(0, 0); // Create new process group
        // Handle input redirection
        if (cmd->input_file != NULL) {
            input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1) {
                // printf("No such file or directory\n");
                exit(1);
            }
            
            // Redirect stdin to the file
            if (dup2(input_fd, STDIN_FILENO) == -1) {
                perror("dup2 failed");
                close(input_fd);
                exit(1);
            }
            
            // Close the original file descriptor
            close(input_fd);
        }
        
        // Handle output redirection
        if (cmd->output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->append_output) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            output_fd = open(cmd->output_file, flags, 0644);
            if (output_fd == -1) {
                // perror("Unable to create file for writing\n");
                exit(1);
            }
            
            // Redirect stdout to the file
            if (dup2(output_fd, STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                close(output_fd);
                exit(1);
            }
            
            // Close the original file descriptor
            close(output_fd);
        }
        // Try to execute the command
        // printf("Executing...\n");
        if (execvp(cmd->args[0], cmd->args) == -1) {
            // perror("exec failed");
            printf("Command not found!\n");
            exit(1);
        }
    } else if (pid < 0) {
        // Fork failed
        perror("fork failed");
    } else {
        // Parent process - wait for child to complete
        fg_pid = pid;
        // wait(NULL);
        tcsetpgrp(STDIN_FILENO, pid);
        int status;
        //Using WUNTRACED to be notified of stopped children
        if(waitpid(pid, &status, WUNTRACED) > 0)
        {
            if(WIFSTOPPED(status))
            {
                printf("\n[%d] Stopped %s\n", job_counter, full_cmd);
                fflush(stdout);
                update_bg_process_list(cmd, pid, 0); // 0 indicates stopped status
            }
        }
        tcsetpgrp(STDIN_FILENO, getpgrp());
        fg_pid = 0;

        fflush(stdout); 
    }
}

//  ############## LLM Generated Code Ends ##############