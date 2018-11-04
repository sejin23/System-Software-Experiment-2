#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    int fd, line, bytes;
    char* fname;
    if(!strcmp(argv[1], "-c")) {
        fname = argv[3];
        bytes = atoi(argv[2]);
        line = -1;
    } else if(!strcmp(argv[1], "-n")) {
            fname = argv[3];
            line = atoi(argv[2]);
    } else {
            fname = argv[1];
            line = 10;
    }
    if((fd = open(fname, O_RDONLY)) < 0) exit(0);
    printf("%d \n", line);
    close(fd);
}