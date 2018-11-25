#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
void stdinhead(int line);

int main(int argc, char** argv) {
    int fd, line, i, st, dp;
    char* fname;
    char word;
    if(argc < 3){
        line = 10;
        if(argc == 1) stdinhead(line);
        fname = argv[1];
        st = 2;
        dp = st;
    }else{
        line = atoi(argv[2]);
        if(argc == 3) stdinhead(line);
        fname = argv[3];
        st = 4;
        dp = st;
    }
    if(argc > st){
        while(argc >= st){
            if(dp != st) write(1, "\n", strlen("\n"));
            write(1, "==> ", strlen("==> "));
            write(1, fname, strlen(fname));
            write(1, " <==\n", strlen(" <==\n"));
            if((fd = open(fname, O_RDONLY)) < 0) exit(0);
            for(i=0;i<line;i++){
                while(1){
                    if(read(fd, &word, 1) == 0) return 0;
                    write(1, &word, sizeof(char));
                    if(word == '\n') break;
                }
            }
            close(fd);
            fname = argv[st];
            st++;
        }
    }else{
        if((fd = open(fname, O_RDONLY)) < 0) exit(0);
        for(i=0;i<line;i++){
            while(1){
                if(read(fd, &word, 1) == 0) return 0;
                write(1, &word, sizeof(char));
                if(word == '\n') break;
            }
        }
        close(fd);
    }
    return 0;
}

void stdinhead(int line){
    char word;
    int i;
    for(i=0;i<line;i++){
        while(1){
            if(read(0, &word, 1) == 0) exit(0);
            write(1, &word, sizeof(char));
            if(word == '\n') break;
        }
    }
    exit(0);
}