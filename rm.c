#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char condir[MAXPATH];
    char buf[MAXPATH];
    if(argc < 2) return 0;
    getcwd(condir, MAXPATH);
    condir[strlen(condir)+1] = '\0';
	condir[strlen(condir)] = '/';
    strcat(condir, argv[1]);
    
    if(unlink(condir) < 0){
        if(errno == EACCES) fprintf(stderr,"rm: Permission denied\n");
        else if(errno == EISDIR) fprintf(stderr,"rm: Is a directory\n");
        else if(errno == ENOENT) fprintf(stderr,"rm: No such file or directory\n");
        else if(errno == ENOTDIR) fprintf(stderr,"rm: Not a directory\n");
        else if(errno == EPERM) fprintf(stderr,"rm: Permission denied\n");
        else{
            sprintf(buf, "rm: Error occurred: <%d>\n", errno);
            write(1, buf, strlen(buf));
        }
    }
    return 0;
}