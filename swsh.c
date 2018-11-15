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
#define MAXPATH 100
#define MAXARGS 128
#define MAXLINE 256
#define MAXPIPE 30
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>

void eval(char *cmdline);
char *which_command(char **argv);
void changedir(char *argv);
void pipeline(char **argv, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void binding(char **argv);
void phandler_int(int sig);
void chandler_int(int sig);
void chandler_stp(int sig);
void ignore(int sig);
char condir[MAXPATH];
pid_t masterpid;
pid_t chdpid, mstpid;
int main(int argc, char **argv)
{
	int i;
	char cmdline[MAXLINE];
	char *ret;
	char *buf = strtok(argv[0], "/swsh");
	realpath(buf, condir);
	i = open("pipe_in.txt", O_CREAT, 0755);
	mstpid = 0;
	masterpid = getpid();
	signal(SIGINT, phandler_int);
	signal(SIGTSTP, phandler_int);
	close(i);
	while (1)
	{
		printf("swsh> ");
		ret = fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin) || ret == NULL)
			exit(0);
		eval(cmdline);
	}
}

void eval(char *cmdline)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int bg;
	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	pipeline(argv, bg);
	return;
}

char *which_command(char **argv)
{
	int fd[2];
	int status;
	char *root = NULL;
	if (strcmp(argv[0], "head") && strcmp(argv[0], "tail") && strcmp(argv[0], "cp") && strcmp(argv[0], "cat") && strcmp(argv[0], "rm") && strcmp(argv[0], "mv") && strcmp(argv[0], "pwd") && strcmp(argv[0], "man"))
		root = strdup("/usr/bin/which");
	else
	{
		root = (char *)malloc(sizeof(char) * MAXLINE);
		strcpy(root, condir);
		strcat(root, "/");
		strcat(root, argv[0]);
		return root;
	}
	pid_t pid;
	if (pipe(fd) < 0)
		exit(1);
	if ((pid = fork()) == 0)
	{
		close(fd[0]);
		dup2(fd[1], 1);
		execl(root, root, argv[0], NULL);
		exit(1);
	}
	else
	{
		if (waitpid(pid, &status, 0) < 0)
			write(2, "waitpid error\n", strlen("waitpid error\n"));
		if (root != NULL)
			free(root);
		root = (char *)malloc(sizeof(char) * MAXLINE);
		close(fd[1]);
		if (read(fd[0], root, MAXLINE) < 0)
			exit(1);
		if (root[strlen(root) - 1] == '\n')
			root[strlen(root) - 1] = '\0';
	}
	return root;
}

int builtin_command(char **argv)
{
	pid_t pid;
	char buf[MAXARGS];
	int status;
	if (!strcmp(argv[0], "quit"))
	{
		if ((pid = fork()) == 0)
		{
			strcpy(buf, condir);
			strcat(buf, "/pipe_in.txt");
			if (execl("/bin/rm", "/bin/rm", buf, NULL) < 0)
				exit(0);
		}
		else
			waitpid(pid, &status, 0);
		exit(0);
	}
	if (!strcmp(argv[0], "&"))
		return 1;
	return 0;
}

int parseline(char *buf, char **argv)
{
	char *delim;
	int argc;
	int bg;
	buf[strlen(buf) - 1] = ' ';
	while (*buf && (*buf == ' '))
		buf++;
	argc = 0;
	while ((delim = strchr(buf, ' ')))
	{
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' '))
			buf++;
	}
	argv[argc] = NULL;
	if (argc == 0)
		return 1;
	if ((bg = (*argv[argc - 1] == '&')) != 0)
		argv[--argc] = NULL;
	return bg;
}

void pipeline(char **argv, int bg)
{
	pid_t pid;
	char *new_argv[MAXARGS];
	char *pipe_in = NULL;
	char *pipe_out = NULL;
	char *temp;
	char buf[MAXARGS];
	char pwdir[MAXPATH];
	int i, j, k, t;
	int status, app, chd = 0, ext = 0, pip = 0, man = 0;
	int fin = 0, fout = 1;
	if (argv[0] == NULL)
		return;
	if (builtin_command(argv))
		return;
	for (i = 0; argv[i] != NULL; i++)
	{
		if (!strcmp(argv[i], "|"))
			pip = 1;
		else if (!strcmp(argv[i], "cd"))
			chd = 1;
		else if (!strcmp(argv[i], "exit"))
			ext = 1;
		else if (!strcmp(argv[i], "man"))
			man = 1;
	}
	if (!pip)
	{
		if (chd == 1)
		{
			if (bg)
			{
				if ((pid = fork()) == 0)
				{
					setpgrp();
					for (i = 0; strcmp(argv[i], "cd"); i++);
					changedir(argv[i + 1]);
					exit(0);
				}
				else
					write(1, "[1] cd\n", strlen("[1] cd\n"));
			}
			else
			{
				for (i = 0; strcmp(argv[i], "cd"); i++);
				changedir(argv[i + 1]);
				return;
			}
		}
		if (ext == 1)
		{
			if (bg)
			{
				if ((pid = fork()) == 0)
				{
					setpgrp();
					for (i = 0; strcmp(argv[i], "exit"); i++);
					write(1, "exit\n", strlen("exit\n"));
					if (argv[i + 1] != NULL)
						exit(atoi(argv[i + 1]));
					else
						exit(0);
				}
			}
			else
			{
				write(1, "exit\n", strlen("exit\n"));
				for (i = 0; strcmp(argv[i], "exit"); i++);
				if ((pid = fork()) == 0){
					strcpy(buf, condir);
					strcat(buf, "/pipe_in.txt");
					if (execl("/bin/rm", "/bin/rm", buf, NULL) < 0)
						exit(0);
				}else waitpid(pid, &status, 0);
				
				if (argv[i + 1] != NULL)
					exit(atoi(argv[i + 1]));
				else
					exit(0);
			}
		}
	}
	if ((mstpid = fork()) == 0)
	{
		if (!man)
			setpgrp();
		signal(SIGINT, chandler_int);
		signal(SIGTSTP, chandler_stp);
		i = 0;
		while (1)
		{
			if (argv[i] == NULL)
				break;
			for (k = 0; argv[i] != NULL && strcmp(argv[i], "|"); i++, k++)
			{
				if (!strcmp(argv[i], "<") || !strcmp(argv[i], ">") || !strcmp(argv[i], ">>"))
					break;
				new_argv[k] = strdup(argv[i]);
			}
			new_argv[k] = NULL;
			binding(new_argv);
			app = 0;
			while (argv[i] != NULL && strcmp(argv[i], "|"))
			{
				if (!strcmp(argv[i], "<"))
					pipe_in = strdup(argv[i + 1]);
				else if (!strcmp(argv[i], ">"))
					pipe_out = strdup(argv[i + 1]);
				else if (!strcmp(argv[i], ">>"))
				{
					pipe_out = strdup(argv[i + 1]);
					app = 1;
				}
				else
					break;
				i += 2;
			}
			if (!strcmp(new_argv[0], "cd") || !strcmp(new_argv[0], "exit"))
			{
				i++;
				continue;
			}
			if ((chdpid = fork()) == 0)
			{
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				if (argv[i] != NULL && !strcmp(argv[i], "|"))
				{
					if ((fout = open("pipe_in.txt", O_CREAT | O_RDWR | O_TRUNC, 0755)) < 0)
						exit(0);
					dup2(fout, 1);
				}
				else if (pipe_out != NULL)
				{
					if (app)
					{
						if ((fout = open(pipe_out, O_CREAT | O_WRONLY | O_APPEND, 0755)) < 0)
							exit(0);
					}
					else
					{
						if ((fout = open(pipe_out, O_CREAT | O_WRONLY | O_TRUNC, 0755)) < 0)
							exit(0);
					}
					dup2(fout, 1);
				}
				if (pipe_in != NULL)
				{
					if ((fin = open(pipe_in, O_RDONLY)) < 0)
						exit(0);
					dup2(fin, 0);
				}
				if (execv(new_argv[0], new_argv) < 0)
				{
					temp = strdup(new_argv[0]);
					new_argv[0] = which_command(new_argv);
					if (execv(new_argv[0], new_argv) < 0)
					{
						fprintf(stderr, "%s: Command not found.\n", temp);
						free(temp);
						exit(0);
					}
				}
			}
			if ((argv[i] == NULL && !bg) || argv[i] != NULL)
			{
				if (waitpid(chdpid, &status, 0) < 0)
					write(1, "waitfd: waitpid error", strlen("waitfd: waitpid error"));
			}
			else if (bg)
			{
				sprintf(buf, "[1] %d\n", chdpid);
				write(1, buf, strlen(buf));
			}
			for (j = 0; new_argv[j] != NULL; j++)
				free(new_argv[j]);
			if (argv[i] == NULL)
				break;
			if (!strcmp(argv[i], "|"))
			{
				if (pipe_in != NULL)
					free(pipe_in);
				pipe_in = strdup("pipe_in.txt");
			}
			i++;
		}
		if (pipe_in != NULL)
			free(pipe_in);
		if (pipe_out != NULL)
			free(pipe_out);
		if (fout != 1)
			close(fout);
		if (fin != 0)
			close(fin);
		exit(0);
	}
	else
	{
		while (waitpid(mstpid, &status, 0) > 0);
		mstpid = 0;
	}
}

void changedir(char *argv)
{
	int i;
	char pwdir[MAXPATH];
	char *user_name;
	struct passwd *u_info;
	u_info = getpwuid(getuid());
	if (argv == NULL)
		strcpy(pwdir, u_info->pw_dir);
	else if (argv[0] == '/')
		strcpy(pwdir, argv);
	else if (argv[0] == '~')
	{
		strcpy(pwdir, u_info->pw_dir);
		if (argv[1] != '\0')
		{
			for (i = 1; argv[i] != '\0'; i++)
				pwdir[strlen(u_info->pw_dir) - 1 + i] = argv[i];
			pwdir[strlen(u_info->pw_dir) - 1 + i] = '\0';
		}
	}
	else
	{
		getcwd(pwdir, MAXPATH);
		pwdir[strlen(pwdir) + 1] = '\0';
		pwdir[strlen(pwdir)] = '/';
		strcat(pwdir, argv);
	}
	if (chdir(pwdir) < 0)
		perror("cd");
}

void binding(char **argv)
{
	int j, k, t;
	char *temp = NULL;
	for (k = 0; argv[k] != NULL; k++)
	{
		if (argv[k][0] == '\'')
		{
			if (argv[k][strlen(argv[k]) - 1] == '\'')
			{
				j = 0;
				temp = strdup(argv[k]);
				break;
			}
			temp = (char *)malloc(sizeof(char) * MAXLINE);
			strcpy(temp, argv[k]);
			for (j = k + 1; argv[j][strlen(argv[j]) - 1] != '\''; j++)
				sprintf(temp, "%s %s", temp, argv[j]);
			sprintf(temp, "%s %s", temp, argv[j]);
			break;
		}
		else if (argv[k][0] == '\"')
		{
			if (argv[k][strlen(argv[k]) - 1] == '\"')
			{
				j = 0;
				temp = strdup(argv[k]);
				break;
			}
			temp = (char *)malloc(sizeof(char) * MAXLINE);
			strcpy(temp, argv[k]);
			for (j = k + 1; argv[j][strlen(argv[j]) - 1] != '\"'; j++)
				sprintf(temp, "%s %s", temp, argv[j]);
			sprintf(temp, "%s %s", temp, argv[j]);
			break;
		}
	}
	if (temp != NULL)
	{
		for (t = 1; temp[t] != '\0'; t++)
			temp[t - 1] = temp[t];
		temp[t - 2] = '\0';
		argv[k] = temp;
		if (j > 0)
		{
			for (t = 1; argv[t + j] != NULL; t++)
			{
				strcpy(argv[t + k], argv[t + j]);
				argv[t + j] = NULL;
			}
			argv[t + k] = NULL;
		}
		temp = NULL;
	}
}

void phandler_int(int sig)
{
	if (mstpid == 0)
		write(1, "\nswsh> ", strlen("\nswsh> "));
	else
		kill(-1 * mstpid, SIGINT);
}

void chandler_int(int sig)
{
	pid_t pid;
	int status;
	while ((pid = waitpid(0, &status, WNOHANG | WUNTRACED)) > 0);
	exit(0);
}

void chandler_stp(int sig)
{
	pid_t pid;
	int status;
	pid = waitpid(0, &status, WUNTRACED);
	kill(pid, SIGINT);
	while (waitpid(pid, &status, WNOHANG));
	exit(0);
}