#include "../include/parser.h"

// ############## LLM Generated Code Begins ##############

// Initialize parser
void init_parser(Parser *p, const char *input) {
    p->input = input;
    p->pos = 0;
    p->length = strlen(input);
}

// Look at current character without consuming it
char peek(Parser *p) {
    if (p->pos >= p->length) {
        return '\0';
    }
    return p->input[p->pos];
}

// Consume and return current character
char consume(Parser *p) {
    if (p->pos >= p->length) {
        return '\0';
    }
    return p->input[p->pos++];
}

// Skip whitespace characters
void skip_whitespace(Parser *p) {
    while (p->pos < p->length && isspace(p->input[p->pos])) {
        p->pos++;
    }
}

// Check if character is valid in a name token
bool is_name_char(char c) {
    return c != '\0' && c != '|' && c != '&' && c != '>' && c != '<' && c != ';' && !isspace(c);
}

// Parse a name token: [^|&><;]+
bool parse_name(Parser *p) {
    skip_whitespace(p);
    int start = p->pos;
    
    // Match characters that are NOT |, &, >, <, ;, or whitespace
    while (p->pos < p->length && is_name_char(p->input[p->pos])) {
        p->pos++;
    }
    
    return p->pos > start;  // Return true if we consumed at least one character
}

// Parse input redirection: < name | <name
bool parse_input(Parser *p) {
    skip_whitespace(p);
    
    if (peek(p) != '<') {
        return false;
    }
    
    consume(p);  // consume '<'
    
    // Parse the filename
    if (!parse_name(p)) {
        return false;
    }
    
    return true;
}

// Parse output redirection: > name | >name | >> name | >>name
bool parse_output(Parser *p) {
    skip_whitespace(p);
    
    if (peek(p) != '>') {
        return false;
    }
    
    consume(p);  // consume first '>'
    
    // Check for '>>' (append)
    if (peek(p) == '>') {
        consume(p);  // consume second '>'
    }
    
    // Parse the filename
    if (!parse_name(p)) {
        return false;
    }
    
    return true;
}

// Parse atomic: name (name | input | output)*
bool parse_atomic(Parser *p) {
    // Must start with a name (command)
    if (!parse_name(p)) {
        return false;
    }
    
    // Parse optional arguments, inputs, and outputs
    while (true) {
        int saved_pos = p->pos;
        
        // Try to parse input redirection
        if (parse_input(p)) {
            continue;
        }
        
        p->pos = saved_pos;
        
        // Try to parse output redirection  
        if (parse_output(p)) {
            continue;
        }
        
        p->pos = saved_pos;
        
        // Try to parse another name (argument)
        if (parse_name(p)) {
            continue;
        }
        
        // Nothing more to parse
        break;
    }
    
    return true;
}

// Parse cmd_group: atomic (\| atomic)*
bool parse_cmd_group(Parser *p) {
    // Parse first atomic command
    if (!parse_atomic(p)) {
        return false;
    }
    
    // Parse optional piped commands
    while (true) {
        skip_whitespace(p);
        int saved_pos = p->pos;
        
        // Check for pipe (single |, not ||)
        if (peek(p) == '|') {
            // Look ahead to make sure it's not ||
            if (p->pos + 1 < p->length && p->input[p->pos + 1] == '|') {
                p->pos = saved_pos;
                break;
            }
            
            consume(p);  // consume '|'
            
            // Must have another atomic after pipe
            if (!parse_atomic(p)) {
                return false;
            }
        } else {
            p->pos = saved_pos;
            break;
        }
    }
    
    return true;
}

// Parse shell_cmd: cmd_group ((& | &&) cmd_group)* &?
// Parse shell_cmd: cmd_group ((& | &&) cmd_group)* &?
bool parse_shell_cmd(Parser *p) {
    // Parse first command group
    if (!parse_cmd_group(p)) {
        return false;
    }
    
    // Parse optional chained command groups
    while (true) {
        skip_whitespace(p);
        int saved_pos = p->pos;
        
        // Check for & or &&
        if (peek(p) == '&') {
            consume(p);  // consume first '&'
            
            // Check if it's && or just &
            skip_whitespace(p);
            // If there's no cmd_group after &, this might be a trailing &
            if (!parse_cmd_group(p)) {
                p->pos = saved_pos;  // Restore position for trailing & check
                break;
            }
            continue;
            // if (peek(p) == '&') {
            // consume(p);  // consume second '&'
            // // Must have another cmd_group after &&
            // }

            // else {
            //     // Single & - could be background operator or separator
            //     // Check if there's another cmd_group after it
            //     int saved_pos2 = p->pos;
            //     skip_whitespace(p);
                
            //     // If we find another &, this is invalid (& &)
            //     if (peek(p) == '&') {
            //         return false;  // Invalid: & &
            //     }
                
            //     // Try to parse another cmd_group
            //     if (parse_cmd_group(p)) {
            //         continue;  // Successfully parsed another cmd_group
            //     } else {
            //         p->pos = saved_pos2;
            //         break;  // No more cmd_groups, & is at end
            //     }
            // }
        }
        else if(peek(p) == ';') {
            consume(p);  // consume ';'
            
            // Must have another cmd_group after ;
            if (!parse_cmd_group(p)) {
                return false;
            }
            continue;
        }

        else {
            p->pos = saved_pos;
            break;
        }
    }
    
    // Check for optional trailing &
    skip_whitespace(p);
    if (peek(p) == '&') {
        // Make sure it's not &&
        if (p->pos + 1 >= p->length || p->input[p->pos + 1] != '&') {
            consume(p);
        }
    }
    
    return true;
}

// Check if the command is valid according to the grammar
bool is_valid_command(const char *input) {
    if (!input || strlen(input) == 0) {
        return false;
    }
    
    Parser p;
    init_parser(&p, input);
    
    bool result = parse_shell_cmd(&p);
    skip_whitespace(&p);
    
    // Should have consumed entire input
    return result && p.pos >= p.length;
}

// Extract command from shell prompt line
char* extract_command(const char* line) {
    if (!line) return NULL;
    
    // Skip shell prompt format: <user@host:path>
    const char* start = line;
    if (*start == '<') {
        // Find the closing >
        while (*start && *start != '>') {
            start++;
        }
        if (*start == '>') {
            start++; // Skip the >
        }
    }
    
    // Skip whitespace after prompt
    while (*start && isspace(*start)) {
        start++;
    }
    
    // If nothing left, return NULL
    if (*start == '\0') {
        return NULL;
    }
    
    // Create a copy of the remaining string
    char* command = malloc(strlen(start) + 1);
    strcpy(command, start);
    
    // Remove trailing whitespace and newlines
    int len = strlen(command);
    while (len > 0 && isspace(command[len-1])) {
        command[--len] = '\0';
    }
    
    return command;
}

// ############## LLM Generated Code ends ##############
