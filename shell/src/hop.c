#include "../include/hop.h"
#include "../include/globals.h"
#include "../include/parser.h"
#include "../include/command_execution.h"
// struct passwd *pw;
// char* prev_dir = NULL;
// Initialize parser
void init_hop_parser(HopParser *p, const char *input) {
    p->input = input;
    p->pos = 0;
    p->length = strlen(input);
}

// Look at current character without consuming it
char hop_peek(HopParser *p) {
    if (p->pos >= p->length) {
        return '\0';
    }
    return p->input[p->pos];
}

// Consume and return current character
char hop_consume(HopParser *p) {
    if (p->pos >= p->length) {
        return '\0';
    }
    return p->input[p->pos++];
}

// Skip whitespace characters
void hop_skip_whitespace(HopParser *p) {
    while (p->pos < p->length && isspace(p->input[p->pos])) {
        p->pos++;
    }
}

// Check if character is valid in a name token (for hop command)
bool is_hop_name_char(char c) {
    // return c != '\0' && !isspace(c);
    return c != '\0' && c != '|' && c != '&' && c != '>' && c != '<' && c != ';' && !isspace(c);
}

// Parse a name token for hop command
bool parse_hop_name(HopParser *p, char* cwd) {
    hop_skip_whitespace(p);
    int start = p->pos;
    
    // Match characters that are not whitespace
    while (p->pos < p->length && is_hop_name_char(p->input[p->pos])) {
        p->pos++;
    }
    
    if (p->pos <= start) {
        return false;  // No characters consumed
    }
    
    // Extract the parsed name into a local variable
    int name_length = p->pos - start;
    char parsed_name[256];  // Local variable to store the name
    char test_path[1024] = {0};  // Local variable to store the test path
    // Copy the name
    strncpy(parsed_name, p->input + start, name_length);
    parsed_name[name_length] = '\0';
    // printf("Parsed hop name: %s\n", parsed_name);

    // Edge case for root directory
    if (strcmp(parsed_name, "/") == 0) {
        strcpy(prev_dir, cwd);
        strcpy(cwd, "/");
        return true;
    }

    strcpy(test_path, cwd);  // Copy current directory to test_path
    if (test_path[strlen(test_path) - 1] != '/') {
        strcat(test_path, "/");
    }
    strcat(test_path, parsed_name);  // Append the parsed name to test_path
    // if(chdir(test_path) == 0)
    // {
    //     strcpy(prev_dir, cwd);  // Store current directory as previous
    //     strcpy(cwd, test_path);  // Update cwd to the new path
    //     // printf("Changed cwd to: %s\n", cwd);
    //     return true;
    // }
    // else {
    //     printf("No such directory!\n");
    //     return false;
    // }
    // Normalize path (remove double slashes)

    char *src = test_path;
    char *dst = test_path;
    bool last_was_slash = false;

    while (*src) {
        if (*src == '/') {
            if (!last_was_slash) {
                *dst++ = *src;
            }
            last_was_slash = true;
        } else {
            *dst++ = *src;
            last_was_slash = false;
        }
        src++;
    }
    *dst = '\0';

    struct stat statbuf;

    // Check if the path exists and get its information
    if (stat(test_path, &statbuf) != 0) {
        printf("No such directory!\n");
        return false;
    }

    // Check if the path is a directory
    if (S_ISDIR(statbuf.st_mode)) {
        strcpy(prev_dir, cwd);       // Store current directory as previous
        strcpy(cwd, test_path);      // Update the cwd string to the new path
        return true;
    } else {
        // printf("No such directory!\n");
        return false;
    }
    // Append to cwd
    // strcpy(prev_dir, cwd);  // Store current directory as previous
    // if (cwd[strlen(cwd) - 1] != '/') {
    //     strcat(cwd, "/");
    // }
    // strcat(cwd, parsed_name);
    // printf("Updated cwd: %s\n", cwd);
    // return true;
}

// Parse special tokens: ~, ., .., -
bool parse_hop_special_token(HopParser *p, char* cwd, char* shell_homedir) {
    hop_skip_whitespace(p);
    
    char current = hop_peek(p);
    
    if (current == '~') {
        hop_consume(p);
        pw = getpwuid(getuid());  // getuid() returns current user ID
        // prev_dir = cwd;
        strcpy(prev_dir, cwd);  // Store current directory as previous
        // cwd = pw->pw_dir;
        // printf("prev_dir: %s\n", prev_dir);
        strncpy(cwd, shell_homedir, strlen(shell_homedir) + 1);
        // printf("Changed cwd to home directory: %s\n", cwd);
        return true;
    }
    
    if (current == '-') {
        char temp[1024] = {0};
        strcpy(temp, cwd);  // Store current directory in temp
        if(strlen(prev_dir) != 0)
        {
            // cwd = prev_dir;
            // prev_dir = temp;
            strncpy(cwd, prev_dir, strlen(prev_dir) + 1);
            strcpy(prev_dir, temp);  // Store current directory as previous
        }
        hop_consume(p);
        return true;
    }

    if (current == '.') {
        if (p->pos + 1 < p->length && p->input[p->pos + 1] == '.') {
            // Potentially ".."
            if (p->pos + 2 < p->length && !isspace(p->input[p->pos + 2])) {
                return false; // It's part of a longer name like "..foo"
            }
            hop_consume(p); // consume first '.'
            hop_consume(p); // consume second '.'
            strcpy(prev_dir, cwd);
            char* last_slash = strrchr(cwd, '/');
            if (last_slash != NULL) {
                if (last_slash == cwd) {
                    cwd[1] = '\0';
                } else {
                    *last_slash = '\0';
                }
            }
            return true;
        } else {
            // Potentially "."
            if (p->pos + 1 < p->length && !isspace(p->input[p->pos + 1])) {
                return false; // It's part of a longer name like ".venv"
            }
            hop_consume(p); // consume '.'
            // This is the "." command, which does nothing to the path.
            return true;
        }
    }
    
    // if (current == '.') {
    //     hop_consume(p);
        
    //     // Check if it's ".." 
    //     if (hop_peek(p) == '.') {
    //         hop_consume(p);  // consume second '.'
    //         strcpy(prev_dir, cwd);  // Store current directory as previous
    //         // Handle ".." - go to parent directory
    //         char* last_slash = strrchr(cwd, '/');
    //         if (last_slash != NULL) {
    //             if (last_slash == cwd) {
    //                 // If the last slash is at the beginning, we're at root
    //                 cwd[1] = '\0';  // Keep just "/"
    //             } else {
    //                 // Remove everything after the last slash
    //                 *last_slash = '\0';
    //             }
    //         }
    //     }
    //     // Both "." and ".." are valid
    //     return true;
    // }
    
    return false;
}

// Parse a single hop argument: (~ | . | .. | - | name)
bool parse_hop_argument(HopParser *p, char* cwd, char* shell_homedir) {
    int saved_pos = p->pos;
    
    // Try to parse special tokens first
    if (parse_hop_special_token(p, cwd, shell_homedir)) {
        return true;
    }
    
    p->pos = saved_pos;
    
    // Try to parse name
    if (parse_hop_name(p, cwd)) {
        // printf("Parsed hop argument: %s\n", cwd);
        return true;
    }
    
    return false;
}

// Parse hop arguments: (~ | . | .. | - | name)*
bool parse_hop_arguments(HopParser *p, char* cwd, char* shell_homedir) {
    // Parse zero or more arguments
    while (true) {
        int saved_pos = p->pos;
        
        if (parse_hop_argument(p, cwd, shell_homedir)) {
            continue;  // Successfully parsed an argument, try for more
        }
        
        p->pos = saved_pos;
        break;  // No more arguments to parse
    }
    // No command after hop so go to home directory
    // printf("Copying previous directory: %s\n", cwd);
    // strcpy(prev_dir, cwd);  // Store current directory as previous
    // printf("Previous directory stored: %s\n", prev_dir);
    // strncpy(cwd, pw->pw_dir, strlen(pw->pw_dir) + 1);
    // printf("Final cwd after hop arguments: %s\n", cwd);
    return true;  // Always succeeds because * means zero or more
}

// Parse the complete hop command: hop ((~ | . | .. | - | name)*)?
bool parse_hop_command(HopParser *p, char* cwd, char* shell_homedir) {
    hop_skip_whitespace(p);
    
    // Must start with "hop"
    if (p->pos + 3 > p->length || strncmp(p->input + p->pos, "hop", 3) != 0) {
        return false;
    }
    
    p->pos += 3;  // consume "hop"
    
    // Check that "hop" is followed by whitespace or end of input
    // This prevents matching "hopeful" as "hop"
    if (p->pos < p->length && !isspace(p->input[p->pos])) {
        return false;
    }
    
    // Parse optional arguments: ((~ | . | .. | - | name)*)?
    // The outer ? makes the entire group optional
    // The inner * makes individual arguments repeatable
    parse_hop_arguments(p, cwd, shell_homedir);
    
    return true;
}

// Check if the command is a valid hop command
bool is_valid_hop_command(const char *input, char*cwd, char* shell_homedir) {
    if (!input || strlen(input) == 0) {
        return false;
    }
    
    HopParser p;
    init_hop_parser(&p, input);
    
    bool result = parse_hop_command(&p, cwd, shell_homedir);
    hop_skip_whitespace(&p);
    
    // Should have consumed entire input
    return result && p.pos >= p.length;
}
char* execute_hop(char* command, char* cwd, char* shell_homedir)
{
    // printf("Executing hop command: %s\n", command);
    char temp_cwd[1024];
    strncpy(temp_cwd, cwd, sizeof(temp_cwd)-1);
    temp_cwd[sizeof(temp_cwd)-1] = '\0';
    
    if(is_valid_hop_command(command, temp_cwd, shell_homedir))
    {
        if (chdir(temp_cwd) == 0) {
            // On success, update the shell's main cwd variable
            strncpy(cwd, temp_cwd, strlen(temp_cwd) + 1);
            return cwd;
        }
    }
    return NULL;
}


char* execute_hop_with_command_t(command_t *cmd, char *cwd, char *shell_homedir) {
    char input_buffer[2048] = {0};
    char temp_buffer[1024] = {0};

    // Step 1: Get input string

    // FILE *f = fopen(cmd->input_file, "r");
    // if (!f) {
    //     perror("Failed to open input file for hop");
    //     return NULL;
    // }
    // // Read first line (or entire file if you want)
    // if (fgets(temp_buffer, sizeof(temp_buffer), f) == NULL) {
    //     fclose(f);
    //     fprintf(stderr, "Input file is empty: %s\n", cmd->input_file);
    //     return NULL;
    // }
    // fclose(f);
    // Step 1: Open file with open()
    // int fd = open(cmd->input_file, O_RDONLY);
    // if (fd < 0) {
    //     perror("Failed to open input file for hop");
    //     return NULL;
    // }

    // // Save current stdin
    // int saved_stdin = dup(STDIN_FILENO);
    // if (saved_stdin == -1) {
    //     perror("dup failed to save stdin");
    //     close(fd);
    //     return NULL;
    // }

    // // Step 2: Redirect STDIN to the opened file
    // if (dup2(fd, STDIN_FILENO) == -1) {
    //     perror("dup2 failed");
    //     close(fd);
    //     close(saved_stdin);
    //     return NULL;
    // }
    // close(fd); // fd no longer needed

    // // Step 3: Read from stdin (now the file)
    // ssize_t bytes = read(STDIN_FILENO, temp_buffer, sizeof(temp_buffer) - 1);
    // if (bytes <= 0) {
    //     fprintf(stderr, "Input file is empty or read failed: %s\n", cmd->input_file);
    //     // Restore stdin before returning
    //     dup2(saved_stdin, STDIN_FILENO);
    //     close(saved_stdin);
    //     return NULL;
    // }
    // temp_buffer[bytes] = '\0'; // null-terminate

    // // Remove trailing newline
    // size_t len = strlen(temp_buffer);
    // if (len > 0 && temp_buffer[len - 1] == '\n') {
    //     temp_buffer[len - 1] = '\0';
    // }

    // snprintf(input_buffer, sizeof(input_buffer), "hop %s", temp_buffer);
    
    // // Step 2: Run through the hop parser
    // char temp_cwd[1024];
    // strncpy(temp_cwd, cwd, sizeof(temp_cwd) - 1);
    // temp_cwd[sizeof(temp_cwd) - 1] = '\0';

    // if (is_valid_hop_command(input_buffer, temp_cwd, shell_homedir)) {
    //     strncpy(cwd, temp_cwd, strlen(temp_cwd) + 1);
    //     return cwd;
    // }
    // return NULL;

    // ############## LLM Generated Code Begins ##############
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return NULL;
    }

    if (pid == 0) {  
        // === Child process ===
        close(pipefd[0]);  // close read end (child only writes)

        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror("Failed to open input file for hop");
            _exit(1);
        }

        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2 failed");
            close(fd);
            _exit(1);
        }
        close(fd);

        // Read input (from redirected stdin)
        ssize_t bytes = read(STDIN_FILENO, temp_buffer, sizeof(temp_buffer) - 1);
        if (bytes <= 0) {
            fprintf(stderr, "Input file is empty or read failed: %s\n", cmd->input_file);
            _exit(1);
        }
        temp_buffer[bytes] = '\0';

        // Remove trailing newline
        size_t len = strlen(temp_buffer);
        if (len > 0 && temp_buffer[len - 1] == '\n') {
            temp_buffer[len - 1] = '\0';
        }

        snprintf(input_buffer, sizeof(input_buffer), "hop %s", temp_buffer);

        char temp_cwd[1024];
        strncpy(temp_cwd, cwd, sizeof(temp_cwd) - 1);
        temp_cwd[sizeof(temp_cwd) - 1] = '\0';

        if (is_valid_hop_command(input_buffer, temp_cwd, shell_homedir)) {
            // Send the updated cwd to the parent via pipe
            write(pipefd[1], temp_cwd, strlen(temp_cwd) + 1);
            close(pipefd[1]);
            _exit(0);
        } else {
            _exit(2);
        }
    } 
    else {  
        // === Parent process ===
        close(pipefd[1]); // close write end (parent only reads)

        char new_cwd[1024] = {0};
        ssize_t n = read(pipefd[0], new_cwd, sizeof(new_cwd) - 1);
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (n > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // ✅ Copy result into cwd (parent’s memory)
            strncpy(cwd, new_cwd, strlen(new_cwd) + 1);
            return cwd;
        }
        return NULL;
    }

    // ################ LLM Generated Code Ends ##############

}
