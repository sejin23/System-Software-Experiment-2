#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXARGS 128
int main(int argc, char** argv){
    char* new_argv[MAXARGS];
    int status, i;
    pid_t pid;
    for(i=0;i<argc-1;i++) new_argv[i+1] = strdup(argv[i+1]);
    new_argv[i+1] = NULL;
    new_argv[0] = strdup("/usr/bin/man");
    for(i=0;new_argv[i]!=NULL;i++) printf("%d : %s\n", i+1, new_argv[i]);    //
    if((pid = fork()) == 0) execv(new_argv[0], new_argv);
    else waitpid(pid, &status, 0);
    return 0;
}