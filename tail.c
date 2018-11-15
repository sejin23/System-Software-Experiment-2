#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 128

int main(int argc, char** argv){
    int fd, line, i, n, st, dp;
    char* fname;
    char word;
    char buf[MAXLINE];
    /*if(argc == 1){

        return 0;
    }*/
    if(!strcmp(argv[1], "-n")) {
        fname = argv[3];
        line = atoi(argv[2]);
        st = 4;
        dp = 4;
    } else {
        fname = argv[1];
        line = 10;
        st = 2;
        dp = 2;
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