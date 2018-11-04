#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    int line = 10;
    if(!strcmp(argv[1], "-c")) {
        printf("1\n");
    } else {
        if(!strcmp(argv[1], "-n")) printf("2");
        printf("3+2");
    }
}