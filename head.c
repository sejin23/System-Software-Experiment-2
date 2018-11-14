#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv) {
    int fd, line, i;
    char* fname;
    char word;
    if(!strcmp(argv[1], "-n")) {
            fname = argv[3];
            line = atoi(argv[2]);
            //if()
    } else {
            fname = argv[1];
            line = 10;
    }
    if((fd = open(fname, O_RDONLY)) < 0) exit(0);
    for(i=0;i<line;i++){
        while(1){
            if(read(fd, &word, 1) == 0) return 0;
            printf("%c", word);
            if(word == '\n') break;
        }
    }
    close(fd);
    return 0;
}