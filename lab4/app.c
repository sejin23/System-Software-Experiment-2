#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
int db_s;
int hash_func(char* str){
	int i, len, ret = 1;
	len = strlen(str);
	for(i=0;i<len;i++){
		ret += str[i];
	}
	return ret%db_s;
}

int main(int argc, char* argv[]) {
	int size, thread_num, i;
	char key[1024];
    db_s = 15;
    scanf("%s", key);
    printf("%d\n", hash_func(key));
	return 0;
}
