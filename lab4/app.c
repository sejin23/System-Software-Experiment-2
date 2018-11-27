#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int db_s;
int hash_func(char* str){
	int i, len, ret = 1;
	len = strlen(str);
	for(i=0;i<len;i++){
		ret *= str[i];
	}
	if(ret%db_s < 0) return ret%db_s+db_s;
	return ret%db_s;
}

int main(int argc, char* argv[]) {
	int i, fd, data, wtp, len, w;
	char buf[1024];
	char dir[64];
	db_s = 128;
	sprintf(dir, "./db/%d.txt", atoi(argv[1]));
	fd = open(dir, O_RDONLY);
	if(fd < 0) exit(0);
	for(i=0;i<=129;i++){
		wtp = read(fd, &w, sizeof(int));
		printf("%d:%d ", i, w);
	}
	printf("\n");
	while(1){
		wtp = read(fd, &len, sizeof(int));
		if(wtp == 0) break;
		wtp = read(fd, buf, len);
		buf[len] = '\0';
		wtp = read(fd, &data, sizeof(int));
		wtp = read(fd, &w, sizeof(int));
		printf("hash: %d, len: %d, key: %s, val: %d, po: %d\n",hash_func(buf), len, buf, data, w);
	}
	wtp = wtp;
	close(fd);
	return 0;
}
