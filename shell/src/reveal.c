#include "../include/reveal.h"
#include "../include/globals.h"
#include "../include/command_execution.h"

// struct passwd *pw;
// char* prev_dir = NULL;

void init_rev_parser(RevParser *p, const char *input)
{
    p->input = input;
    p->pos = 0;
    p->length = strlen(input);
}

// Look at current character without consuming it
char rev_peek(RevParser *p)
{
    if (p->pos >= p->length)
    {
        return '\0';
    }
    return p->input[p->pos];
}

// Consume and return current character
char rev_consume(RevParser *p)
{
    if (p->pos >= p->length)
    {
        return '\0';
    }
    return p->input[p->pos++];
}

// Skip whitespace characters
void rev_skip_whitespace(RevParser *p)
{
    while (p->pos < p->length && isspace(p->input[p->pos]))
    {
        p->pos++;
    }
}

bool parse_rev_name(RevParser *p, char *cwd, bool a_flag, bool l_flag, char *output_file)
{
    rev_skip_whitespace(p);
    int start = p->pos;
    bool append = false;
    // Match characters that are not whitespace
    while (p->pos < p->length && !isspace(p->input[p->pos]) && p->input[p->pos] != '<')
    {
        if (p->input[p->pos] == '>')
        {
            if (p->input[p->pos + 1] == '>')
            {
                append = true; // Found ">>" for append
                break;
            }
            break;
        }
        else
        {
            p->pos++;
        }
    }

    // if (p->pos <= start) {
    //     return false;  // No characters consumed
    // }

    // Extract the parsed name into a local variable
    int name_length = p->pos - start;
    char parsed_name[256];      // Local variable to store the name
    char test_path[1024] = {0}; // Local variable to store the test path
    if (name_length > 0)
    {
        strncpy(parsed_name, p->input + start, name_length);
        parsed_name[name_length] = '\0';

        // printf("Parsed reveal name: %s\n", parsed_name);

        strcpy(test_path, cwd);
        if (test_path[strlen(test_path) - 1] != '/')
        {
            strcat(test_path, "/");
        }
        strcat(test_path, parsed_name);
    }
    else
    {
        strcpy(test_path, cwd);
    }
    // ############## LLM Generated Code Begins ##############
    struct stat statbuf;
    if (stat(test_path, &statbuf) != 0)
    {
        printf("No such directory!\n");
        return false;
    }

    if (S_ISDIR(statbuf.st_mode))
    {
        // It's a directory, proceed with the original logic to list contents.

        DIR *dir;
        struct dirent *entry;

        dir = opendir(test_path);
        if (dir == NULL)
        {
            printf("No such directory!\n");
            return false;
        }
        // Store filenames first to sort them
        char *filenames[1024];
        int file_count = 0;
        int max_width = 0;

        // printf("Contents of directory %s:\n", test_path);
        while ((entry = readdir(dir)) != NULL)
        {
            // Skip . and .. entries always
            if (!a_flag && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))
            {
                continue;
            }

            // Skip hidden files unless a_flag is true
            if (!a_flag && entry->d_name[0] == '.')
            {
                continue;
            }

            // Allocate and store filename
            filenames[file_count] = strdup(entry->d_name);
            int len = strlen(entry->d_name);
            if (len > max_width)
            {
                max_width = len;
            }
            file_count++;
        }

        // Sort filenames
        for (int i = 0; i < file_count - 1; i++)
        {
            for (int j = 0; j < file_count - i - 1; j++)
            {
                if (strcmp(filenames[j], filenames[j + 1]) > 0)
                {
                    char *temp = filenames[j];
                    filenames[j] = filenames[j + 1];
                    filenames[j + 1] = temp;
                }
            }
        }
        if (l_flag)
        {
            if (output_file == NULL)
            {
                for (int i = 0; i < file_count; i++)
                {
                    printf("%s\n", filenames[i]);
                }
            }
            else
            {
                FILE *fp = fopen(output_file, "w");
                if (fp == NULL)
                {
                    perror("Error opening output file");
                    closedir(dir);
                    return false;
                }
                for (int i = 0; i < file_count; i++)
                {
                    fprintf(fp, "%s\n", filenames[i]);
                }
                fclose(fp);
            }
        }
        else
        {
            // Calculate columns and spacing
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            int term_width = w.ws_col;

            int col_width = max_width + 2; // Add 2 for spacing
            int num_cols = term_width / col_width;
            if (num_cols == 0)
                num_cols = 1;

            // Print files in columns
            if (output_file == NULL)
            {
                for (int i = 0; i < file_count; i++)
                {
                    //              %s Print a string
                    //              - Left-align the text (pad with spaces on the right)
                    //              * Width is specified by an argument (the col_width variable)
                    //              So printf("%-*s", col_width, filenames[i]) prints the filename left-aligned in a field of col_width characters
                    printf("%-*s", col_width, filenames[i]);
                    if ((i + 1) % num_cols == 0 || i == file_count - 1)
                    {
                        printf("\n");
                    }
                }
            }
            else
            {
                FILE *fp;
                if (append == false)
                {
                    fp = fopen(output_file, "w");
                }
                else
                    fp = fopen(output_file, "a");
                if (fp == NULL)
                {
                    perror("Error opening output file");
                    closedir(dir);
                    return false;
                }
                for (int i = 0; i < file_count; i++)
                {
                    fprintf(fp, "%-*s", col_width, filenames[i]);
                    if ((i + 1) % num_cols == 0 || i == file_count - 1)
                    {
                        fprintf(fp, "\n");
                    }
                }
                fclose(fp);
            }
        }

        // Free allocated memory
        for (int i = 0; i < file_count; i++)
        {
            free(filenames[i]);
        }

        closedir(dir);
    }
    return true;

    // ############## LLM Generated Code Ends ##############
}

// Parse special tokens: ~, ., .., -
bool parse_rev_special_token(RevParser *p, char *cwd, char *shell_homedir, bool a_flag, bool l_flag, char *output_file)
{
    rev_skip_whitespace(p);

    char current = rev_peek(p);
    // printf("Current char in reveal: %c\n", current);
    if (current == '~')
    {
        rev_consume(p);
        pw = getpwuid(getuid()); // getuid() returns current user ID
        // prev_dir = cwd;
        strcpy(prev_dir, cwd); // Store current directory as previous
        // cwd = pw->pw_dir;
        // printf("prev_dir: %s\n", prev_dir);
        strncpy(cwd, shell_homedir, strlen(shell_homedir) + 1);
        // printf("Showing files in: %s\n", cwd);
        return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
        // return true;
    }

    // if (current == '-')
    // {
    //     char temp[1024] = {0};
    //     strcpy(temp, cwd); // Store current directory in temp
    //     if (strlen(prev_dir) != 0)
    //     {
    //         // cwd = prev_dir;
    //         // prev_dir = temp;
    //         strncpy(cwd, prev_dir, strlen(prev_dir) + 1);
    //         strcpy(prev_dir, temp); // Store current directory as previous
    //     }
    //     else {
    //         printf("No such directory!\n");
    //         return false;
    //     }
    //     rev_consume(p);
    //     return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
    //     // return true;
    // }
    if (current == '-') {
        // rev_skip_whitespace(p);
        // Ensure it's a standalone '-'
        if (p->pos + 1 < p->length && !isspace(p->input[p->pos + 1])) {
            return false;
        }
        rev_consume(p);
        // printf("Previous directory: %s\n", prev_dir);
        if(strlen(prev_dir) == 0 ) {
            printf("No such directory!\n");
            return false;
        }
        // printf("%s\n", prev_dir);
        char temp[1024] = {0};
        strcpy(temp, cwd);
        
        strncpy(cwd, prev_dir, strlen(prev_dir) + 1);
        strcpy(prev_dir, temp);
        
        return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
    }

    // if (current == '.')
    // {
    //     rev_consume(p);

    //     // Check if it's ".."
    //     if (rev_peek(p) == '.')
    //     {
    //         rev_consume(p);        // consume second '.'
    //         strcpy(prev_dir, cwd); // Store current directory as previous
    //         // Handle ".." - go to parent directory
    //         char *last_slash = strrchr(cwd, '/');
    //         if (last_slash != NULL)
    //         {
    //             if (last_slash == cwd)
    //             {
    //                 // If the last slash is at the beginning, we're at root
    //                 cwd[1] = '\0'; // Keep just "/"
    //             }
    //             else
    //             {
    //                 // Remove everything after the last slash
    //                 *last_slash = '\0';
    //             }
    //         }
    //         return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
    //     }
    //     return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
    //     // Both "." and ".." are valid
    //     // return true;
    // }
    if (current == '.') {
        if (p->pos + 1 < p->length && p->input[p->pos + 1] == '.') {
            // Potentially ".."
            if (p->pos + 2 < p->length && !isspace(p->input[p->pos + 2])) {
                return false; // It's part of a longer name like "..foo"
            }
            rev_consume(p); // consume first '.'
            rev_consume(p); // consume second '.'
            strcpy(prev_dir, cwd);
            char *last_slash = strrchr(cwd, '/');
            if (last_slash != NULL) {
                if (last_slash == cwd) cwd[1] = '\0';
                else *last_slash = '\0';
            }
            return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
        } else {
            // Potentially "."
            if (p->pos + 1 < p->length && !isspace(p->input[p->pos + 1])) {
                return false; // It's part of a longer name like ".venv"
            }
            rev_consume(p); // consume '.'
            return parse_rev_name(p, cwd, a_flag, l_flag, output_file);
        }
    }

    return false;
}

// parse a single argument for reveal command
// It can be a special token (~, ., .., -) or a name
// Parse a single directory argument (not flag)
bool parse_rev_directory_argument(RevParser *p, char *cwd, char *shell_homedir, bool a_flag, bool l_flag, char *output_file)
{
    int saved_pos = p->pos;

    // Try to parse special tokens first
    if (parse_rev_special_token(p, cwd, shell_homedir, a_flag, l_flag, output_file))
    {
        return true;
    }

    p->pos = saved_pos;

    // Try to parse name
    if (parse_rev_name(p, cwd, a_flag, l_flag, output_file))
    {
        // printf("Parsed rev argument: %s\n", cwd);
        return true;
    }

    return false;
}

// // Parse reveal arguments: (~ | . | .. | - | name)*
// bool parse_rev_arguments(RevParser *p, char* cwd, char* shell_homedir, bool a_flag, bool l_flag) {
//     // Parse zero or more arguments
//     while (true) {
//         int saved_pos = p->pos;

//         if (parse_rev_argument(p, cwd, shell_homedir, a_flag, l_flag)) {
//             continue;  // Successfully parsed an argument, try for more
//         }

//         p->pos = saved_pos;
//         break;  // No more arguments to parse
//     }

//     return true;  // Always succeeds because * means zero or more
// }

bool parse_rev_flags(RevParser *p, bool *a_flag, bool *l_flag)
{
    rev_skip_whitespace(p);

    if (p->pos >= p->length || p->input[p->pos] != '-')
    {
        return false; // Not a flag.
    }
    if (p->pos + 1 >= p->length || isspace(p->input[p->pos + 1])) {
        return false;
    }

    rev_consume(p); // consume '-'

    // Parse flag characters
    bool found_flag = false;
    while (p->pos < p->length && (p->input[p->pos] == 'a' || p->input[p->pos] == 'l'))
    {
        if (p->input[p->pos] == 'a')
        {
            *a_flag = true;
            found_flag = true;
        }
        else if (p->input[p->pos] == 'l')
        {
            *l_flag = true;
            found_flag = true;
        }
        p->pos++;
    }

    return found_flag;
}

bool parse_rev_command(RevParser *p, char *cwd, char *shell_homedir, char *output_file)
{
    rev_skip_whitespace(p);

    // Must start with "reveal"
    if (p->pos + 6 > p->length || strncmp(p->input + p->pos, "reveal", 6) != 0)
    {
        return false;
    }

    p->pos += 6; // consume "reveal"

    // Check that "reveal" is followed by whitespace or end of input
    // This prevents matching "revealing" as "reveal"
    if (p->pos < p->length && !isspace(p->input[p->pos]))
    {
        return false;
    }
    bool a_flag = false;
    bool l_flag = false;

    while (parse_rev_flags(p, &a_flag, &l_flag))
    {
        // Continue parsing flags until no more are found
    }

    rev_skip_whitespace(p);
    if (p->pos < p->length)
    {
        // There's more input, try to parse directory argument
        if (!parse_rev_directory_argument(p, cwd, shell_homedir, a_flag, l_flag, output_file))
        {
            return false; // Failed to parse directory argument
        }
    }
    else
    {
        // No directory argument, just list current directory
        parse_rev_name(p, cwd, a_flag, l_flag, output_file);
    }

    return true;
    // int saved_pos = p->pos;
    // while(1)
    // {
    //     rev_skip_whitespace(p);

    //     // Parse arguments
    //     if(p->pos < p->length && p->input[p->pos] == '-') {
    //         // Check for flags
    //         rev_consume(p);  // consume '-'
    //         while(p->pos < p->length && (p->input[p->pos] == 'a' || p->input[p->pos] == 'l')) {
    //             if(p->input[p->pos] == 'a') {
    //                 a_flag = true;
    //             }
    //             else if(p->input[p->pos] == 'l') {
    //                 l_flag = true;
    //             }
    //             p->pos++;
    //         }
    //     }
    //     else {
    //         break;
    //     }

    // }
    // p->pos = saved_pos;  // Reset position to start of arguments
    // return parse_rev_arguments(p, cwd, shell_homedir, a_flag, l_flag);
}

void execute_reveal(char *command, char *cwd, char *shell_homedir, char *output_file)
{
    RevParser p;
    init_rev_parser(&p, command);
    char temp[256] = {0};
    strcpy(temp, cwd); // Store current directory in temp
    if (parse_rev_command(&p, temp, shell_homedir, output_file))
    {
        // printf("Reveal command executed successfully.\n");
    }
    else
    {
        printf("reveal: Invalid Syntax!\n");
    }
}

void execute_reveal_with_command_t(command_t *cmd, char *cwd, char *shell_homedir)
{
    char input_buffer[2048] = {0};
    char temp_buffer[1024] = {0};
    // Step 1: Get input string
    // FILE *f = fopen(cmd->input_file, "r");
    // if (!f) {
    //     perror("Failed to open input file for reveal");
    //     return;
    // }
    // // Read first line (or entire file if you want)
    // if (fgets(temp_buffer, sizeof(temp_buffer), f) == NULL) {
    //     fclose(f);
    //     fprintf(stderr, "Input file is empty: %s\n", cmd->input_file);
    //     return;
    // }
    // fclose(f);

    // // Remove trailing newline
    // size_t len = strlen(temp_buffer);
    // if (len > 0 && temp_buffer[len - 1] == '\n') {
    //     temp_buffer[len - 1] = '\0';
    // }

    // snprintf(input_buffer, sizeof(input_buffer), "reveal %s", temp_buffer);

    // // Step 2: Run through the reveal parser
    // char temp_cwd[1024];
    // strncpy(temp_cwd, cwd, sizeof(temp_cwd) - 1);
    // temp_cwd[sizeof(temp_cwd) - 1] = '\0';

    // execute_reveal(input_buffer, temp_cwd, shell_homedir, cmd->output_file);
    // strncpy(cwd, temp_cwd, strlen(temp_cwd) + 1);
    // return;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        return;
    }
    if (pid == 0)
    {
        // === Child process ===
        // printf("%s\n", cmd->input_file);
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0)
        {
            perror("Failed to open input file for reveal");
            _exit(1);
        }

        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("dup2 failed");
            close(fd);
            _exit(1);
        }
        close(fd);

        // Read input (from redirected stdin)
        ssize_t bytes = read(STDIN_FILENO, temp_buffer, sizeof(temp_buffer) - 1);
        if (bytes <= 0)
        {
            fprintf(stderr, "Input file is empty or read failed: %s\n", cmd->input_file);
            _exit(1);
        }
        temp_buffer[bytes] = '\0';

        // Remove trailing newline
        size_t len = strlen(temp_buffer);
        if (len > 0 && temp_buffer[len - 1] == '\n')
        {
            temp_buffer[len - 1] = '\0';
        }

        snprintf(input_buffer, sizeof(input_buffer), "reveal %s", temp_buffer);

        char temp_cwd[1024];
        strncpy(temp_cwd, cwd, sizeof(temp_cwd) - 1);
        temp_cwd[sizeof(temp_cwd) - 1] = '\0';

        execute_reveal(input_buffer, temp_cwd, shell_homedir, cmd->output_file);
        strncpy(cwd, temp_cwd, strlen(temp_cwd) + 1);
        _exit(0);
    }
    else
    {
        // === Parent process ===
        int status;
        waitpid(pid, &status, 0);
    }
}
