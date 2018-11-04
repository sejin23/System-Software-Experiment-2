#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 128

int main(int argc, char** argv){
    int n, fd1, fd2;
    char buf[MAXLINE];
    if((fd1 = open(argv[1], O_RDONLY)) < 0) return 0;
    if((fd2 = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, 0755)) < 0) return 0;

    memset(buf, 0, MAXLINE);
    while(n = read(fd1, buf, MAXLINE)){
        write(fd2, buf, n);
        memset(buf, 0, MAXLINE);
    }
    close(fd1);
    close(fd2);
}