#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char condir[MAXPATH];
    char pwdir[MAXPATH];
    if(argc < 3) return 0;
    getcwd(condir, MAXPATH);
    condir[strlen(condir)+1] = '\0';
	condir[strlen(condir)] = '/';
    strcpy(pwdir, condir);
    strcat(condir, argv[1]);
    strcat(pwdir, argv[2]);
    
    if(rename(condir, pwdir) < 0) perror("mv");
    return 0;
}