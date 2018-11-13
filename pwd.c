#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char pwdir[MAXPATH];
    getcwd(pwdir, MAXPATH);
    printf("%s\n", pwdir);
    return 0;
}