#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "smsh_parse.h"

/**
 * Function to expand "$$" into the current PID.
 */
char *expand_pid(char *str);

/**
 *  Parses a string of user input into a command structure.
 */
struct smsh_command smsh_parse(char* input)
{
    /** initialize result */
    struct smsh_command result;
    result.name = NULL;
    result.argc = 0;
    result.argv = NULL;
    result.input_file = NULL;
    result.output_file = NULL;
    result.bg = 0;

    // parse the string into a command structure
    int input_len = strlen(input);
    if(input_len > 0)
    {
        /* allocate space for max args (512), split input on whitespace */
        char* args[512];
        char *tok = strtok(input, " ");
        int numTok = 0;
        while(tok != NULL)
        {
            args[numTok] = (char*)malloc(strlen(tok) + 1);
            strcpy(args[numTok], tok);
            numTok++;
            tok = strtok(NULL, " ");
        }

        if(numTok > 0)
        {
            /** only continue processing if not a comment */
            if(args[0][0] != '#')
            {
                /** first token is the command name, not included in args */
                result.argc = numTok + 1;
                result.argv = calloc(result.argc, sizeof(char*));
                
                /** copy the name of the command */
                result.name = (char*)malloc(strlen(args[0] + 1));
                strcpy(result.name, args[0]);

                /** copy name into arguments list */
                result.argv[0] = (char*)malloc(strlen(result.name) + 1);
                strcpy(result.argv[0], result.name);

                int i = 0;
                int num_ord_args = 1;
                for(i = 1; i < numTok; i++)
                {
                    if(strcmp(args[i], "<") == 0)
                    {
                        /** input file is next */
                        /** make sure there is another argument to interpret as the redirect target */
                        if(i + 1 < numTok)
                        {
                            i++;
                            char *replaced = expand_pid(args[i]);
                            int len = strlen(replaced) + 1;
                            result.input_file = (char*)malloc(len);
                            strcpy(result.input_file, replaced);
                            result.input_file[len-1] = '\0';
                        }
                    }
                    else if(strcmp(args[i], ">") == 0)
                    {
                        /** output file is next */
                        /** make sure there is another argument to interpret as the redirect target */
                        if(i + 1 < numTok)
                        {
                            i++;
                            char *replaced = expand_pid(args[i]);
                            int len = strlen(replaced) + 1;
                            result.output_file = (char*)malloc(len);
                            strcpy(result.output_file, replaced);
                            result.output_file[len-1] = '\0';
                        }
                    }
                    else
                    {
                        /** ordinary argument */
                        char *replaced = expand_pid(args[i]);
                        int len = strlen(replaced) + 1;
                        result.argv[num_ord_args] = (char*)malloc(len * sizeof(char));
                        strcpy(result.argv[num_ord_args], replaced);
                        result.argv[num_ord_args][len - 1] = '\0';
                        num_ord_args++;
                    }
                }
                result.argc = num_ord_args;

                if(strcmp(result.argv[result.argc - 1], "&") == 0)
                {
                    /** if last arg is "&" set background flag and remove las argument */
                    result.bg = 1;
                    free(result.argv[result.argc - 1]);
                    result.argv[result.argc-1] = NULL;
                    result.argc = result.argc-1;
                }
            }
        }
    }

    return result;
}

/**
 * Expands "$$" into the current process ID.
 */
char *expand_pid(char *str)
{
    int pid = getpid();
    char pid_string[10];
    sprintf(pid_string, "%d", pid);

    static char buffer[4096];
    char *p;

    if(!(p = strstr(str, "$$")))
        return str;

    strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
    buffer[p-str] = '\0';

    sprintf(buffer+(p-str), "%s%s", pid_string, p+strlen("$$"));

    return buffer;
}
