#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv){
    execv(argv[1], argv);
    return 0;
}