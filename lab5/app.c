#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_KEYLEN 1024
int main(){
    int i, j, len;
    char buf[MAX_KEYLEN];
    char key[MAX_KEYLEN];
    char* val;
    while(1){
        memset(buf, 0, MAX_KEYLEN);
        if(fgets(buf, MAX_KEYLEN, stdin) != NULL){
            if(!strncmp(buf, "GET", strlen("GET"))){
                for(i=strlen("GET "),j=0;buf[i]!='\n';i++){
                    if(buf[i] == ' ') continue;
                    key[j] = buf[i];
                    j++;
                }
                key[j] = '\0';
                printf("GET: %s--\n", key);
            }else if(!strncmp(buf, "PUT", strlen("PUT"))){
                for(i=strlen("PUT "),j=0; buf[i]!=32;i++,j++)
                    key[j] = buf[i];
                key[j] = '\0';
                i++;
                val = (char*)malloc(strlen(buf)-i);
                for(j=0;buf[i]!='\n';i++,j++)
                    val[j] = buf[i];
                val[j] = '\0';
                printf("PUT: %s--%s\n", key, val);
                free(val);
            }
        }
    }
}