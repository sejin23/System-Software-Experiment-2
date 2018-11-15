#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 128
#define MAXWORD 1048576
void stdintail(int line);
int main(int argc, char** argv){
    int fd, line, i, n, st, dp;
    char* fname;
    char word;
    char buf[MAXLINE];
    if(argc < 3){
        line = 10;
        if(argc == 1) stdintail(line);
        fname = argv[1];
        st = 2;
        dp = 2;
    }else{
        line = atoi(argv[2]);
        if(argc == 3) stdintail(line);
        fname = argv[3];
        st = 4;
        dp = 4;
    }
    if(argc > st){
        while(argc >= st){
            if(dp != st) write(1, "\n", strlen("\n"));
            write(1, "==> ", strlen("==> "));
            write(1, fname, strlen(fname));
            write(1, " <==\n", strlen(" <==\n"));
            if((fd = open(fname, O_RDONLY)) < 0) return 0;
            lseek(fd, -1, SEEK_END);
            for(i=0;i<line;i++){
                while(n = read(fd, &word, 1)){
                    lseek(fd, -2, SEEK_CUR);
                    if(word == '\n') break;
                }
            }
            lseek(fd, 2, SEEK_CUR);
            memset(buf, 0, MAXLINE);
            while(n = read(fd, buf, MAXLINE)){
                write(1, buf, n);
                if(n < MAXLINE) break;
            }
            close(fd);
            fname = argv[st];
            st++;
        }
    }else{
        if((fd = open(fname, O_RDONLY)) < 0) return 0;
        lseek(fd, -1, SEEK_END);
        for(i=0;i<line;i++){
            while(n = read(fd, &word, 1)){
                lseek(fd, -2, SEEK_CUR);
                if(word == '\n') break;
            }
        }
        lseek(fd, 2, SEEK_CUR);
        memset(buf, 0, MAXLINE);
        while(n = read(fd, buf, MAXLINE)){
            write(1, buf, n);
            if(n < MAXLINE) break;
        }
        close(fd);
    }
    return 0;
}

void stdintail(int line){
    char buf[MAXWORD];
    int i, j, n;
    j = 0;
    n = 0;
    memset(buf, 0, MAXWORD);
    while(1){
        read(0, &buf[n], 1);
        if(buf[n] == EOF || n == MAXWORD - 1) break;
        n++;
    }
    for(i=0;i<line;i++){
        while(1){
            if(n-j == 0) break;
            else j++;
            if(buf[n-j] == '\n') break;
        }
    }
    for(i=n-j+1;i<n;i++) write(1, &buf[i], sizeof(char));
    exit(0);
}