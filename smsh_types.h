#ifndef SMSH_TYPES_H
#define SMSH_TYPES_H

/**
 * Command structure.
 */
struct smsh_command
{
    char*  name;
    int    argc;
    char** argv;
    char*  input_file;
    char*  output_file;
    int    bg;
};

/**
 * Child process structure, used to stor background child info.
 */
struct smsh_child
{
    int pid;
    int term_sig;
    int exit_code;
};

#endif
