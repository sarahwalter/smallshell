#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "smsh_exec.h"
#include "smsh_print.h"
#include "smsh_child_list.h"

pid_t fg_pid = 0;

void smsh_exec(struct smsh_command cmd)
{
    static int last_fg_exit = 0;
    static int last_fg_sig = 0;
    if(cmd.name == NULL)
    {
        /** no command, this will happen if the command 
         *  is a blank line or comment */
    }
    else if(strcmp(cmd.name, "cd") == 0)
    {
        if(cmd.argc == 1)
        {
            last_fg_exit = chdir(getenv("HOME"));
        }
        else
        {
            last_fg_exit = chdir(cmd.argv[1]);
        }
    }
    else if(strcmp(cmd.name, "status") == 0)
    {
        if(last_fg_exit == 0 && last_fg_sig != 0)
        {
            smsh_printf("terminated by signal %d\n", last_fg_sig);
        }
        else
        {
            smsh_printf("exit value %d\n", last_fg_exit);
        }
    }
    else
    {
        int pid = fork();
        if(pid == 0)
        {
            /* set default signal handler for sigint) */
            struct sigaction sigint_action;
            if(cmd.bg == 1)
            {
                /* background child, ignore SIGINT */
                sigint_action.sa_handler = SIG_IGN;
            }
            else
            {
                /* foreground child, handle SIGINT by default */
                sigint_action.sa_handler = SIG_DFL;
            }
            sigint_action.sa_flags = 0;
            sigaction(SIGINT, &sigint_action, NULL);

            /** set up redirections */
            int in_fd, out_fd;
            if(cmd.input_file)
            {
                if((in_fd = open(cmd.input_file, O_RDONLY)) == -1)
                {
                    perror(NULL);
                    exit(1);
                }
                if(dup2(in_fd, STDIN_FILENO) == -1)
                {
                    perror(NULL);
                    exit(1);
                }
                close(in_fd);
            }
            else
            {
                if(cmd.bg == 1)
                {
                    if((in_fd = open("/dev/null", O_RDONLY)) == -1)
                    {
                        perror(NULL);
                        exit(1);
                    }
                    if(dup2(in_fd, STDIN_FILENO) == -1)
                    {
                        perror(NULL);
                        exit(1);
                    }
                    close(in_fd);
                }
            }

            if(cmd.output_file)
            {
                if((out_fd = creat(cmd.output_file,  0644)) == -1)
                {
                    perror(NULL);
                    exit(1);
                }

                if(dup2(out_fd, STDOUT_FILENO) == -1)
                {
                    perror(NULL);
                    exit(1);
                }

                close(out_fd);
            }
            else
            {
                if(cmd.bg)
                {
                    if((out_fd = creat("/dev/null",  0644)) == -1)
                    {
                        perror(NULL);
                        exit(1);
                    }

                    if(dup2(out_fd, STDOUT_FILENO) == -1)
                    {
                        perror(NULL);
                        exit(1);
                    }

                    close(out_fd);
                }
            }

            /** execute the command */
            if(execvp(cmd.name, cmd.argv) == -1)
            {
                perror("exec failure");
                exit(1);
            }

            /** close input and output */
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
        }
        else
        {
            if(cmd.bg == 0)
            {
                fg_pid = pid;
                /** We want to handle the SIGINT of the child process here.
                 *  So set the parent to ignore SIGINT until after the fg
                 *  child completes */
                struct sigaction new_action, old_action;
                new_action.sa_handler = SIG_IGN;
                sigemptyset(&new_action.sa_mask);
                new_action.sa_flags = 0;
                sigaction(SIGINT, &new_action, &old_action);

		        int status;
                waitpid(fg_pid, &status, 0);
                int sig = WTERMSIG(status);
                if(sig > 0)
                {
                    last_fg_sig = sig;
                    last_fg_exit = 0;
                    printf("terminated by signal %d\n", sig);
                }
                else
                {
                    last_fg_sig = 0;
                    last_fg_exit = WEXITSTATUS(status);
                }

                /** set back to previous SIGINT handler */
                sigaction(SIGINT, &old_action, NULL);
            }
            else
            {
                struct smsh_child new_child;
                new_child.pid = pid;
                new_child.term_sig = 0;
                new_child.exit_code = 0;
                add_child(&new_child);
                fg_pid = 0;
                smsh_printf("background pid is %d\n", pid);
            }
        }
    }
}
