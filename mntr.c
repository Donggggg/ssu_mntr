#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#define FILELEN 100
#define BUFLEN 1024
#define TRUE 1
#define FALSE 0
typedef struct file_status{
	char name[FILELEN];
	struct dirent **namelist;
	struct stat statbuf;
	struct file_status *down;
	struct file_status *next;
}file_stat;

file_stat* make_tree(char *path);
file_stat* all_nodes(file_stat* head);

int main(void)
{
	FILE* fp;
	DIR *dirp;
	struct dirent **namelist; // 디렉토리의 파일 리스트
	struct dirent **tmplist;
	struct stat statbuf;
	int i, count;
	char *filename, saved_path[BUFLEN];
	char tmp[BUFLEN];
	file_stat *head = malloc(sizeof(file_stat));

	if(access("check", F_OK) < 0) // 모니터링할 디렉토리 없을 시 생성
		mkdir("check", 0755);

	if((fp = fopen("log.txt", "w+")) < 0){ // 모니터링 기록을 남길 파일 생성
		fprintf(stderr, "fopen error for log.txt\n");
		exit(1);
	}

	if((dirp = opendir("check")) == NULL){ // 모니터링할 디렉토리 open
		fprintf(stderr, "error for open dir[test]\n");
		exit(1);
	}

	memset(saved_path, 0, BUFLEN);
	getcwd(saved_path, BUFLEN);
	sprintf(saved_path, "%s/%s", saved_path, "check");
	head = make_tree(saved_path);
	//	printf("%s\n", head->down->next->next->next->next->next->down->name);
	all_nodes(head);

	exit(0);

}

file_stat* make_tree(char *path)
{
	int i, count;
	int isFirst = TRUE;
	char tmp[BUFLEN];
	file_stat *head = malloc(sizeof(file_stat));
	file_stat *now = malloc(sizeof(file_stat));
	now = head;
	strcpy(head->name, path);

	if((count = scandir(head->name, &(head->namelist), NULL, alphasort)) == -1){
		fprintf(stderr, "scandir error\n");
		exit(1);
	}

	for(i = 0;i < count; i++){
		if(!strcmp(head->namelist[i]->d_name, ".") || !strcmp(head->namelist[i]->d_name, "..")) // "."와  ".." 스킵
			continue;

		file_stat *new = malloc(sizeof(file_stat));
		new->down = NULL;
		new->next = NULL;
		strcpy(new->name, head->namelist[i]->d_name);
		sprintf(tmp, "%s/%s", path, new->name);
		strcpy(new->name, tmp);

		if(stat(tmp, &(new->statbuf)) < 0){
			fprintf(stderr, "stat error\n");
			exit(1);
		}

		if(S_ISDIR(new->statbuf.st_mode)){ // 디렉토리면
			new = make_tree(tmp);
		}
		else{ // 파일이면
		}

		if(isFirst == TRUE){
			now->down = new;
			now = now->down;
			isFirst = FALSE;
		}
		else{
			now->next = new;
			now = now->next;
		}

		//	free(new);
	} 
	return head;
}

file_stat* all_nodes(file_stat* head)
{
	file_stat *now = malloc(sizeof(file_stat));
	now = head->down;
	printf("%s\n", head->name);

	while(1){
		printf("%s\n", now->name);
		if(now->down != NULL){
			now = all_nodes(now);
			now = now->next;
			if(now == NULL)
				break;
		}
		else if(now->next != NULL){
			now = now->next;
		}
		else
			break;
	}
	return head;
}
