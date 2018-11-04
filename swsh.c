//-----------------------------------------------------------
//
// SSE2033 : System Software Experiment 2 (Fall 2018)
//
// Skeleton Code for PA#3
//
// Nov 1, 2018.
// CSLab, Sungkyunkwan University
//
// Forked by http://csapp.cs.cmu.edu/public/ics2/code/ecf/shellex.c
//-----------------------------------------------------------


/* $begin shellmain */
#define MAXARGS   128
#define MAXLINE	  256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/* function prototypes */
void eval(char *cmdline);
char* which_command(char** argv);
void pipeline(char** argv, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

int main()
{
  char cmdline[MAXLINE]; /* Command line */
	char* ret;
  while (1) {
		/* Read */
		printf("swsh> ");
		ret = fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin) || ret == NULL)
				exit(0);
		/* Evaluate */
		eval(cmdline);
  } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char* argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	
	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	pipeline(argv, bg);
	return;
}

char* which_command(char** argv){
	int fd[2];
	char* str = "/usr/bin/which";
	char* line = (char*)malloc(sizeof(char)*MAXLINE);
	strcpy(line, argv[0]);

	pid_t pid;
	if(pipe(fd) < 0) exit(1);
	if((pid = fork()) == 0){
		close(fd[0]);
		dup2(fd[1], 1);
		execl(str, str, argv[0], NULL);
		exit(1);
	} else {
		close(fd[1]);
		if(read(fd[0], line, MAXLINE) < 0) exit(1);
		if(line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
	}
	return line;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit")) /* quit command */
			exit(0);  
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
			return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
			buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
			argv[argc++] = buf;
			*delim = '\0';
			buf = delim + 1;
			while (*buf && (*buf == ' ')) /* Ignore spaces */
				buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
			return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
			argv[--argc] = NULL;

    return bg;
}
/* $end parseline */
void pipeline(char** argv, int bg) {
	char* new_argv[MAXARGS];
	char* pipe_in[2];
	int i, j, k, status, fd;
	pid_t pid;
	if(argv[0] == NULL) return;
	k = -1;
	for(i=0;argv[i] != NULL && strcmp(argv[i], "|");i++)
		new_argv[i] = strdup(argv[i]);
	new_argv[i] = NULL;

	while (!builtin_command(new_argv)) {
		if ((pid = fork()) == 0) {
			if(argv[i] != NULL && !strcmp(argv[i], "|")) {
				if((fd = open("pipe_in.txt", O_CREAT|O_RDWR, 0755)) < 0) exit(0);
				dup2(fd, 1);
			}
			if(k >= 0){
				new_argv[k] = strdup(pipe_in[0]);
				new_argv[k+1] = strdup(pipe_in[1]);
				free(pipe_in[0]);
				free(pipe_in[1]);
				new_argv[k+2] = NULL;
			}
			if (execv(new_argv[0], new_argv) < 0) {
				new_argv[0] = which_command(new_argv);
				if(execv(new_argv[0], new_argv) < 0) {
					fprintf(stderr,"%s: Command not found.\n", new_argv[0]);
					exit(0);
				}
			}
		}
		for(j=0;new_argv[j] != NULL;j++) free(new_argv[j]);
		if((argv[i] == NULL && !bg) || argv[i] != NULL) {
			if (waitpid(pid, &status, 0) < 0)
				printf("waitfg: waitpid error");
		} else if(bg)
			printf("background\n");
		
		if(argv[i] != NULL && !strcmp(argv[i], "|")) {
			for(j=i+1;argv[j] != NULL && strcmp(argv[j], "|");j++)
				new_argv[j-i-1] = strdup(argv[j]);
			new_argv[j-i-1] = NULL;
			k = j-i-1;
			i = j;
			pipe_in[0] = strdup("<");
			pipe_in[1] = strdup("pipe_in.txt");
		} else break;
	}
}