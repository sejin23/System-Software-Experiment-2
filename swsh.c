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
#define MAXPATH	  100
#define MAXARGS   128
#define MAXLINE	  256
#define MAXPIPE	  30
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* function prototypes */
void eval(char *cmdline);
char* which_command(char** argv);
void changedir(char* argv);
void pipeline(char** argv, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void binding(char** argv);
void handler(int sig);
void ignore(int sig);
char condir[MAXPATH];
pid_t masterpid;

int main(){
	char cmdline[MAXLINE]; /* Command line */
	char* ret;
	getcwd(condir, MAXPATH);
	masterpid = getpid();
	//signal(SIGINT, ignore);
	//signal(SIGTSTP, ignore);
	//signal(SIGCHLD, handler);
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
	int status;
	char* root = NULL;
	if(strcmp(argv[0], "head") && strcmp(argv[0], "tail") && strcmp(argv[0], "cp") && strcmp(argv[0], "cat") && strcmp(argv[0], "rm") && strcmp(argv[0], "mv") && strcmp(argv[0], "pwd"))
		root = strdup("/usr/bin/which");
	else{
		root = (char*)malloc(sizeof(char)*MAXLINE);
		strcpy(root, condir);
		strcat(root, "/");
		strcat(root, argv[0]);
		return root;
	}
	pid_t pid;
	if(pipe(fd) < 0) exit(1);
	if((pid = fork()) == 0){
		close(fd[0]);
		dup2(fd[1], 1);
		execl(root, root, argv[0], NULL);
		exit(1);
	} else {
		if(waitpid(pid, &status, 0) < 0) printf("waitpid error\n");
		if(root != NULL) free(root);
		root = (char*)malloc(sizeof(char)*MAXLINE);
		close(fd[1]);
		if(read(fd[0], root, MAXLINE) < 0) exit(1);
		if(root[strlen(root)-1] == '\n') root[strlen(root)-1] = '\0';
	}
	return root;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv){
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
	char* temp;
	char pwdir[MAXPATH];
	int i, j, k, t;
	int status, app = 0, pip = 0;
	int fin = 0, fout = 1;
	pid_t pid, mstpid;
	if(argv[0] == NULL) return;
	if(builtin_command(argv)) return;
	for(i=0;argv[i]!=NULL;i++){
		if(!strcmp(argv[i], "|")) pip = 1;
	}
	if(pip == 1){
		if((mstpid = fork()) == 0){
			setpgrp();
			printf("child1 pid : [%d], ppid : [%d], pgid : [%d]\n", getpid(), getppid(), getpgrp());
			i = 0;
			while(1) {
				if(argv[i] == NULL) break;
				for(k = 0;argv[i] != NULL && strcmp(argv[i], "|");i++){
					if(!strcmp(argv[i], "<") || !strcmp(argv[i], ">") || !strcmp(argv[i], ">>")) break;
					new_argv[k] = strdup(argv[i]);
					k++;
				}
				new_argv[k] = NULL;
				binding(new_argv);

				while(argv[i] != NULL && strcmp(argv[i], "|")){
					if(!strcmp(argv[i], "<")) pipe_in = strdup(argv[i+1]);
					else if(!strcmp(argv[i], ">")) pipe_out = strdup(argv[i+1]);
					else if(!strcmp(argv[i], ">>")) {
						pipe_out = strdup(argv[i+1]);
						app = 1;
					} else break;
					i += 2;
				}
				if(!strcmp(new_argv[0], "cd") || !strcmp(new_argv[0], "exit")){
					i++;
					continue;
				}
				if ((pid = fork()) == 0){
					if(masterpid == getppid()) setpgrp();
					printf("child2 pid : [%d], ppid : [%d], pgid : [%d]\n", getpid(), getppid(), getpgrp());
					if(argv[i] != NULL && !strcmp(argv[i],"|")){
						if((fout = open("pipe_in.txt", O_CREAT|O_RDWR|O_TRUNC, 0755)) < 0) exit(0);
						dup2(fout, 1);
					}else if(pipe_out != NULL){
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
						temp = (char*)malloc(sizeof(char)*strlen(new_argv[0])+1);
						strcpy(temp, new_argv[0]);
						new_argv[0] = which_command(new_argv);
						if(execv(new_argv[0], new_argv) < 0) {
							fprintf(stderr,"%s: Command not found.\n", temp);
							free(temp);
							exit(0);
						}
					}
				}
				if((argv[i] == NULL && !bg) || argv[i] != NULL) {
					if (waitpid(pid, &status, 0) < 0)
						printf("waitfg: waitpid error");
				} else if(bg)
					printf("[%d] %s\n", pid, new_argv[0]);

				for(j=0;new_argv[j] != NULL;j++) free(new_argv[j]);
				if(argv[i] == NULL) break;
				if(!strcmp(argv[i], "|")){
					if(pipe_in != NULL) free(pipe_in);
					pipe_in = strdup("pipe_in.txt");
				}
				i++;
			}
			if(pipe_in != NULL) free(pipe_in);
			if(pipe_out != NULL) free(pipe_out);
			if(fout != 1) close(fout);
			if(fin != 0) close(fin);
			exit(0);
		}else{
			printf("parent pid : [%d], ppid : [%d], pgid : [%d]\n", getpid(), getppid(), getpgrp());
			pid = waitpid(mstpid, &status, WUNTRACED);
		}
	}else{
		if(!strcmp(argv[0], "exit")){
			if(bg){
				if((pid = fork()) == 0){
					setpgrp();
					if(argv[1] == NULL){
						printf("exit\n");
						exit(0);
					}else{
						printf("exit [%d]\n", atoi(argv[1]));
						exit(atoi(argv[1]));
					}
				}else printf("[%d] %s\n", pid, argv[0]);
				return;
			}else{
				if(argv[1] == NULL){
					printf("exit\n");
					exit(0);
				}else{
					printf("exit [%d]\n", atoi(argv[1]));
					exit(atoi(argv[1]));
				}
			}
		}
		if(!strcmp(argv[0], "cd")){
			if(bg){
				if((pid = fork()) == 0){
					changedir(argv[1]);
					exit(0);
				}else printf("[%d] %s\n", pid, argv[0]);
				return;
			}else{
				changedir(argv[1]);
				return;
			}
		}
		for(i=0;argv[i]!=NULL;i++){
			if(!strcmp(argv[i], "<") || !strcmp(argv[i], ">") || !strcmp(argv[i], ">>")) break;
			else new_argv[i] = strdup(argv[i]);
		}
		new_argv[i] = NULL;
		for(j=i;argv[j]!=NULL;j++){
			if(!strcmp(argv[j], "<")) pipe_in = strdup(argv[j+1]);
			else if(!strcmp(argv[j], ">")) pipe_out = strdup(argv[j+1]);
			else if(!strcmp(argv[j], ">>")){
				pipe_out = strdup(argv[j+1]);
				app = 1;
			}
			j++;
		}
		binding(new_argv);
		for(j=0;new_argv[j]!=NULL;j++) printf("%d : %s\n", j+1, new_argv[j]);
		if((pid = fork()) == 0){
			if(pipe_out != NULL){
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
				temp = (char*)malloc(sizeof(char)*strlen(new_argv[0])+1);
				strcpy(temp, new_argv[0]);
				new_argv[0] = which_command(new_argv);
				if(execv(new_argv[0], new_argv) < 0) {
					fprintf(stderr,"%s: Command not found.\n", temp);
					free(temp);
					exit(0);
				}
			}
		}else{
			if(!bg){
				if (waitpid(pid, &status, 0) < 0) printf("waitfg: waitpid error");
			}else printf("[%d] %s\n", pid, new_argv[0]);
			for(j=0;new_argv[j] != NULL;j++) free(new_argv[j]);
			if(pipe_in != NULL) free(pipe_in);
			if(pipe_out != NULL) free(pipe_out);
			if(fout != 1) close(fout);
			if(fin != 0) close(fin);
		}
	}
}

void changedir(char* argv){
	int i;
	char pwdir[MAXPATH];
	char* user_name;
	struct passwd* u_info;
	u_info = getpwuid(getuid());
	if(argv == NULL) strcpy(pwdir, u_info->pw_dir);
	else if(argv[0] == '/') strcpy(pwdir, argv);
	else if(argv[0] == '~'){
		strcpy(pwdir, u_info->pw_dir);
		if(argv[1] != '\0'){
			for(i=1;argv[i]!='\0';i++) pwdir[strlen(u_info->pw_dir)-1+i] = argv[i];
			pwdir[strlen(u_info->pw_dir)-1+i] = '\0';
		}
	}else{
		getcwd(pwdir, MAXPATH);
		pwdir[strlen(pwdir)+1] = '\0';
		pwdir[strlen(pwdir)] = '/';
		strcat(pwdir, argv);
	}
	if(chdir(pwdir) < 0) perror("cd");
}

void binding(char** argv){
	int j, k, t;
	char* temp = NULL;
	for(k=0;argv[k]!=NULL;k++){
		if(argv[k][0] == '\''){
			if(argv[k][strlen(argv[k])-1] == '\''){
				j = 0;
				temp = strdup(argv[k]);
				break;
			}
			temp = (char*)malloc(sizeof(char)*MAXLINE);
			strcpy(temp, argv[k]);
			for(j=k+1;argv[j][strlen(argv[j])-1] != '\'';j++)
				sprintf(temp, "%s %s", temp, argv[j]);
			sprintf(temp, "%s %s", temp, argv[j]);
			break;
		}else if(argv[k][0] == '\"'){
			if(argv[k][strlen(argv[k])-1] == '\"'){
				j = 0;
				temp = strdup(argv[k]);
				break;
			}
			temp = (char*)malloc(sizeof(char)*MAXLINE);
			strcpy(temp, argv[k]);
			for(j=k+1;argv[j][strlen(argv[j])-1] != '\"';j++)
				sprintf(temp, "%s %s", temp, argv[j]);
			sprintf(temp, "%s %s", temp, argv[j]);
			break;
		}
	}
	if(temp != NULL){
		for(t=1;temp[t]!='\0';t++)
			temp[t-1] = temp[t];
		temp[t-2] = '\0';
		argv[k] = temp;
		if(j > 0){
			for(t=1;argv[t+j]!=NULL;t++){
				strcpy(argv[t+k], argv[t+j]);
				argv[t+j] = NULL;
			}
			argv[t+k] = NULL;
		}
		temp = NULL;
	}
}

void handler(int sig){
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0);
	if(WIFSTOPPED(status)){
		while((pid = waitpid(0, &status, WNOHANG)) > 0);
	}
}

void ignore(int sig){
	char conexe[MAXPATH];
	printf("\n");
	sprintf(conexe, "%s/", condir);
	strcat(conexe, "swsh");
	execl(conexe, conexe, NULL);
}