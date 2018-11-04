#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 128

int main(int argc, char** argv){
    int fd, n;
    char buf[MAXLINE];
    if((fd = open(argv[1], O_RDONLY)) < 0) return 0;
    memset(buf, 0, MAXLINE);
    while(n = read(fd, buf, MAXLINE)){
        printf("%s", buf);
        memset(buf, 0, MAXLINE);
    }
    close(fd);
}