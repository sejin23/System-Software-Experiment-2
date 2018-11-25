#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAXPATH 100

int main(int argc, char** argv){
    char pwdir[MAXPATH];
    getcwd(pwdir, MAXPATH);
    write(1, pwdir, strlen(pwdir));
    write(1, "\n", strlen("\n"));
    return 0;
}