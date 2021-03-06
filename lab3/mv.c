#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#define MAXARGS 128
int main(int argc, char **argv)
{
    char condir[MAXARGS];
    char pwdir[MAXARGS];
    if(argc < 3) return 0;
    getcwd(condir, MAXARGS);
    condir[strlen(condir)+1] = '\0';
	condir[strlen(condir)] = '/';
    strcpy(pwdir, condir);
    strcat(condir, argv[1]);
    strcat(pwdir, argv[2]);
    rename(condir, pwdir);
    return 0;
}