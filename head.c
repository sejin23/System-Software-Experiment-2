#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv) {
    int fd, line, i, st, dp;
    char* fname;
    char word;
    if(!strcmp(argv[1], "-n")) {
            fname = argv[3];
            line = atoi(argv[2]);
            st = 4;
            dp = st;
    } else {
            fname = argv[1];
            line = 10;
            st = 2;
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
            fname = argv[st];
            st++;
        }
    }else{
        if((fd = open(fname, O_RDONLY)) < 0) exit(0);
        for(i=0;i<line;i++){
            while(1){
                if(read(fd, &word, 1) == 0) return 0;
                printf("%c", word);
                if(word == '\n') break;
            }
        }
        close(fd);
    }
    return 0;
}