#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "smsh_types.h"
#include "smsh_child_list.h"
#include "smsh_exec.h"
#include "smsh_parse.h"
#include "smsh_print.h"

#define MAX_COMMAND_LEN 2048

/** variables for non-local jumps due to SIGINT and SIGTSTP */
static sigjmp_buf env_sigint;
static sigjmp_buf env_sigtstp;

/** falg variable to enable or disable background processes */
static int allow_bg = 1;

/** SIGINT handler. executes non-local jump */
void sigint_handler(int sig)
{
    siglongjmp(env_sigint, 2);
}

/** SIGTSTP handler. executes non-local jump */
void sigtstp_handler(int sig)
{
    siglongjmp(env_sigtstp, 3);
}

/**
 * returns a line of input from stdin
 */
void get_input(char *buffer, int size);

/**
 * Main entry point.
 */
int main(int argc, char** argv)
{
    int retVal = 0;

    /** install signal handlers for SIGINT and SIGSTP */
    struct sigaction sigint_action;
    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);

    struct sigaction sigtstp_action;
    sigtstp_action.sa_handler = sigtstp_handler;
    sigemptyset(&sigtstp_action.sa_mask);
    sigtstp_action.sa_flags = 0;
    sigaction(SIGTSTP, &sigtstp_action, NULL);

    /** command buffer */
    char buffer[MAX_COMMAND_LEN];

    /** loop until break */
    while(1)
    {
        /** If we recieved a SIGINT */
        if(sigsetjmp(env_sigint, 1) == 2)
        {
            smsh_printf("\n");
        }

        /** If we received a SIGTSP */
        if(sigsetjmp(env_sigtstp, 1) == 3)
        {
            // if we received a sigtstp
            if(allow_bg == 1)
            {
                smsh_printf("\nEntering foreground-only mode (& is now ignored)\n");
                allow_bg = 0;
            }
            else
            {
                smsh_printf("\nExiting foreground-only mode\n"); 
                allow_bg = 1;
            }
        }

        /** check background processes */
        struct smsh_child* bg_child;
        while((bg_child = check_children()) != NULL)
        {
            smsh_printf("background pid %d is done: ", bg_child->pid);
            if(bg_child->term_sig > 0)
            {
                /** terminated by signal */
                smsh_printf("terminated by signal %d\n", bg_child->term_sig);
            }
            else
            {
                /** process exited */
                smsh_printf("exit value %d\n", bg_child->exit_code);
            }

            /** bg_child is malloc'd by check_children, we need to free() */
            free(bg_child);
        }
        
        /** prompt, get, and parse user input*/
        smsh_printf(": ");
        get_input(buffer, MAX_COMMAND_LEN);
        struct smsh_command cmd = smsh_parse(buffer);

        /** only allow bg process if flag is set */
        if(allow_bg == 0)
        {
            cmd.bg = 0;
        }

        /** check for "exit", break out of loop if needed */
        if(cmd.name != NULL)
        {
            if(strcmp(cmd.name, "exit") == 0)
            {
                break;
            }
        }

        smsh_exec(cmd);
    }

    return retVal;
}

/**
 * gets user input using fgets.
 */
void get_input(char* buffer, int size)
{
    memset(buffer, 0, size);
    if(fgets(buffer, size, stdin) != NULL)
    {
        buffer[strlen(buffer)-1] = '\0';
    }
}
