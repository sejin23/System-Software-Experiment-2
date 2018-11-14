#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#define MAXPATH 100
#define MAXARGS 256
char* foldir(char* argv);

int main(int argc, char** argv){
    int i;
    char buf[MAXARGS];
    char* pwdir[MAXPATH];
    struct stat statbuf;

    for(i=0;i<argc-1;i++) pwdir[i] = foldir(argv[i+1]);

    if(stat(pwdir[argc-2], &statbuf) < 0){
        perror("stat error");
        exit(0);
    }    

    if(argc > 3){
        for(i=0;i<argc-2;i++){
            if(S_ISDIR(statbuf.st_mode)) sprintf(buf, "%s/%s", pwdir[argc-2], argv[i+1]);
            else{
                printf("mv: Not a directory\n");
                exit(0);
            }
            printf("- %s\n",buf);
            if(rename(pwdir[i], buf) < 0){
                if(errno == EACCES || errno == EISDIR || errno == ENOENT || errno == ENOTDIR || errno == EPERM) perror("mv");
                else printf("mv: Error occurred: <%d>\n",errno);
                exit(0);
            }
        }
    }
    return 0;
}

char* foldir(char* argv){
	int i;
	char* pwdir = (char*)malloc(sizeof(char)*MAXPATH);
    char temp[MAXPATH];
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
        if(argv[0] == '.' && argv[1] == '/'){
            for(i=0;argv[i+2]!='\0';i++) temp[i] = argv[i+2];
            temp[i] = '\0';
        }else strcpy(temp, argv);
		pwdir[strlen(pwdir)+1] = '\0';
		pwdir[strlen(pwdir)] = '/';
		strcat(pwdir, temp);
	}
    return pwdir;
}