#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 128

int main(int argc, char** argv){
    int fd, n;
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    if(argc > 1){
        if((fd = open(argv[1], O_RDONLY)) < 0) return 0;
        while(n = read(fd, buf, MAXLINE)){
            write(1, buf, n);
            memset(buf, 0, MAXLINE);
        }
        close(fd);
    }else{
        while(n = read(0, buf, MAXLINE)){
            write(1, buf, n);
            memset(buf, 0, MAXLINE);
        }
    }
}