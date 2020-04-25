#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(){
	struct stat statbuf;



	if(stat("2.c", &statbuf)<0){
		fprintf(stderr,"1");
		exit(1);
	}
