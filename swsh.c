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
//
// Student ID : 2014313299
// Student Name : Park Sejin
//
//-----------------------------------------------------------


/* $begin shellmain */
#define MAXPATH	  64
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
void changedir(char* argv);
void pipeline(char** argv, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
char condir[MAXPATH];

int main()
{
	char cmdline[MAXLINE]; /* Command line */
	char* ret;
	getcwd(condir, MAXPATH);
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
	char* pipe_in = NULL;
	char* pipe_out = NULL;
	char pwdir[MAXPATH];
	int i, j, k;
	int status, app = 0;
	int fin = 0, fout = 1;
	pid_t pid;

	if(argv[0] == NULL) return;
	i = 0;
	while (1) {
		for(k = 0;argv[i] != NULL && strcmp(argv[i], "|");i++){
			if(!strcmp(argv[i], "<") || !strcmp(argv[i], ">") || !strcmp(argv[i], ">>")) break;
			new_argv[k] = strdup(argv[i]);
			k++;
		}
		new_argv[k] = NULL;
		if(!strcmp(new_argv[0], "cd"))
			changedir(new_argv[1]);

		while(argv[i] != NULL && strcmp(argv[i], "|")){
			if(!strcmp(argv[i], "<")) pipe_in = strdup(argv[i+1]);
			else if(!strcmp(argv[i], ">")) pipe_out = strdup(argv[i+1]);
			else if(!strcmp(argv[i], ">>")) {
				pipe_out = strdup(argv[i+1]);
				app = 1;
			} else break;
			i += 2;
		}
		if(builtin_command(new_argv)) return;
		if ((pid = fork()) == 0) {
			if(argv[i] != NULL && !strcmp(argv[i], "|")) {
				if((fout = open("pipe_in.txt", O_CREAT|O_RDWR|O_TRUNC, 0755)) < 0) exit(0);
				dup2(fout, 1);
			} else if(pipe_out != NULL) {
				if(app) {
					if((fout = open(pipe_out, O_CREAT|O_WRONLY|O_APPEND, 0755)) < 0) exit(0);
				} else {
					if((fout = open(pipe_out, O_CREAT|O_WRONLY|O_TRUNC, 0755)) < 0) exit(0);
				}
				dup2(fout, 1);
			}
			if(pipe_in != NULL) {
				if((fin = open(pipe_in, O_RDONLY)) < 0) exit(0);
				dup2(fin, 0);
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
			if(pipe_in != NULL) free(pipe_in);
			pipe_in = strdup("pipe_in.txt");
		} else break;
		i++;
	}
	if(pipe_in != NULL) free(pipe_in);
	if(pipe_out != NULL) free(pipe_out);
	if(fout != 1) close(fout);
	if(fin != 0) close(fin);
}

void changedir(char* argv){
	char* cd_argv[MAXARGS];
	char* new_argv[MAXARGS];
	char* ret;
	char* rootdir = condir;
	char cmdline[MAXLINE];
	char dir[MAXPATH] = "swsh";
	char pwdir[MAXPATH];
	int bg, root, dirlen, i = 0;

	getcwd(pwdir, MAXPATH);
	pwdir[strlen(pwdir)+1] = '\0';
	pwdir[strlen(pwdir)] = '/';
	strcat(pwdir, argv);
	if(chdir(pwdir) < 0) return;
	memset(pwdir, 0, MAXPATH);
	getcwd(pwdir, MAXPATH);

	root = strcmp(rootdir, pwdir);
	if(root > 0){
		dirlen = strlen(pwdir);
		for(i=0;rootdir[dirlen + i] != '\0';i++){
			if(rootdir[dirlen + i] == '/') strcat(dir, "/..");
		}
	} else if(root < 0) {
		dirlen = strlen(rootdir);
		for(i=0;pwdir[dirlen + i] != '\0';i++){
			dir[4+i] = pwdir[dirlen + i];
		}
		dir[4+i] = '\0';
	}

	while (1) {
		printf("%s> ", dir);
		ret = fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin) || ret == NULL)
			exit(0);
		//bg = parseline(cmdline, cd_argv);
		//for(i=0;cd_argv[i] != NULL;i++) printf("%d : %s\n", i+1, cd_argv[i]);
		eval(cmdline);
	}
}