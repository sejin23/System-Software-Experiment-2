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
#include <sys/types.h>
#include <sys/wait.h>

/* function prototypes */
void eval(char *cmdline);
char* which_command(char** argv);
void pipeline(char** argv, int bg, char* pipe_in);
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
	pipeline(argv, bg, NULL);
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
void pipeline(char** argv, int bg, char* pipe_in) {
	char* new_argv[MAXARGS];
	char pipe_out[MAXLINE];
	int i, j, status;
	int fd[2];
	int fdp[2];
	pid_t pid;
	if (argv[0] == NULL) return;
	for(i=0;argv[i] != NULL && strcmp(argv[i], "|");i++)
		new_argv[i] = strdup(argv[i]);
	new_argv[i] = NULL;

	while (!builtin_command(argv)) {
		if(pipe(fd) < 0) exit(0);

		if ((pid = fork()) == 0) {
			if(argv[i] != NULL && !strcmp(argv[i], "|")) {
				close(fd[0]);
				dup2(fd[1], 1);
			}
			if(pipe_in != NULL){
				if(pipe(fdp) < 0) exit(0);
				if(write(fdp[1], pipe_in, sizeof(pipe_in)) < 0) exit(0);
				dup2(fdp[0], 0);
			}
			if (execv(new_argv[0], new_argv) < 0) {
				scanf("%s", pipe_out);
				printf(" => %s \n", pipe_out);
				dup2(0, fdp[0]);
				new_argv[0] = which_command(argv);
				if(execv(new_argv[0], new_argv) < 0) {
					fprintf(stderr,"%s: Command not found.\n", new_argv[0]);
					exit(0);
				}
				
			}
		}
		for(j=0;j<i;j++) free(new_argv[j]);
		if((argv[i] == NULL && !bg) || argv[i] != NULL) {
			if (waitpid(pid, &status, 0) < 0)
				printf("waitfg: waitpid error");
		} else if(bg)
			printf("background\n");
		
		if(argv[i] != NULL && !strcmp(argv[i], "|")) {
			close(fd[1]);
			if(read(fd[0], pipe_out, MAXLINE) < 0) exit(0);
			for(j=i+1;argv[j] != NULL && strcmp(argv[j], "|");j++)
				new_argv[j-i-1] = strdup(argv[j]);
			new_argv[j-i-1] = NULL;
			pipeline(new_argv, bg, pipe_out);
		}
	}
}