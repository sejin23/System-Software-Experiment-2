#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char condir[MAXPATH];
    if(argc < 2) return 0;
    getcwd(condir, MAXPATH);
    condir[strlen(condir)+1] = '\0';
	condir[strlen(condir)] = '/';
    strcat(condir, argv[1]);
    
    if(unlink(condir) < 0){
        if(errno == EACCES || errno == EISDIR || errno == ENOENT || errno == ENOTDIR || errno == EPERM) perror("rm");
        else printf("rm: Error occurred: <%d>\n",errno);
    }
    return 0;
}