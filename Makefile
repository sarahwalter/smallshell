CC=gcc

all:
	$(CC) smsh_main.c smsh_parse.c smsh_exec.c smsh_print.c smsh_child_list.c -o smallsh
debug:
	$(CC) -ansi -g smsh_main.c smsh_parse.c smsh_exec.c smsh_print.c smsh_child_list.c -o smallsh

clean:
	rm smallsh
	rm *.o


