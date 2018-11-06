#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char condir[MAXPATH];
    if(argc < 2) return 0;
    getcwd(condir, MAXPATH);
    condir[strlen(condir)+1] = '\0';
	condir[strlen(condir)] = '/';
    strcat(condir, argv[1]);
    
    if(unlink(condir) < 0) printf("delete error\n");

    return 0;
}