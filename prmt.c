#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mntr.h"

//#define BUFLEN 1024
//#define MAXNUM 100
void print_tree(file_stat *node, int level, int *length, int *check);
void to_lower_case(char *str);
void print_usage();

int main(void)
{
	int count, direc_length[MAXNUM];
	int lastfile_check[MAXNUM];
	char command_line[BUFLEN], path[BUFLEN];
	char command_tokens[BUFLEN][MAXNUM];
	char *command, *tmp;
	file_stat *head = malloc(sizeof(file_stat));

	while(1)
	{
		printf("20162443>");
		fgets(command_line, BUFLEN, stdin);
		command_line[strlen(command_line)-1] = '\0';

		count = 0;
		command = strtok(command_line, " ");
		to_lower_case(command);

		while((tmp = strtok(NULL, " ")) != NULL)
			strcpy(command_tokens[count++], tmp);

		memset(path, 0, BUFLEN);
		getcwd(path, BUFLEN);
		sprintf(path, "%s/%s", path, "check");

		if(!strcmp(command, "delete"))
		{

		}
		else if(!strcmp(command, "size")){}
		else if(!strcmp(command, "recover")){}
		else if(!strcmp(command, "tree")) // tree명령어 수행 
		{
			head = make_tree(path);
			direc_length[0] = 5;
			lastfile_check[0] = TRUE;
			printf("check------");
			print_tree(head->down, 1, direc_length, lastfile_check);
			printf("\n");
		}
		else if(!strcmp(command, "exit")){}
		else
		{
			print_usage();
		}
	}
}

void print_tree(file_stat *node, int level, int *length, int *check)
{
	int i, j, isFirst=TRUE, max = 6;
	char *filename, *tmp;
	file_stat *now = malloc(sizeof(file_stat));
	now = node;
	check[level-1] = FALSE;

	while(1)
	{
		filename = strtok(now->name, "/");
		while((tmp = strtok(NULL, "/")) != NULL)
			strcpy(filename, tmp);

		max = 6;
		if(isFirst == TRUE){
			printf("|");
			isFirst = FALSE;
		}
		else{
			for(i=0; i<level; i++){
				for(j = 0; j < length[i]; j++)
					printf(" ");
				if(i != 0)
					max = 7;
				for(j = 0; j < max; j++)
					printf(" ");
				if(check[i] == FALSE)
					printf("|");
				else
					printf(" ");
			}
		}
		//sleep(1);

		if(now->next == NULL){
			check[level-1]=TRUE;
		}
		printf("-%s", filename);
		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->down != NULL){
				printf("------");
				length[level] = strlen(filename);
				print_tree(now->down, level+1, length, check);
			}
		}

		if(now->next != NULL){
			now = now->next;
			printf("\n");
		}
		else
			break;
	}
}

void to_lower_case(char *str)
{
	int i = 0;

	while(str[i]){
		if(str[i] >= 'A' && str[i] <= 'Z'){
			str[i] = str[i]+32;
		}
		i++;
	}
}

void print_usage()
{
	printf("----------------------------------------------------------\n");
	printf("			<Usage>\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>DELETE [FILENAME] [END_TIME] [OPTION]\n");
	printf(" delete files at END_TIME automatically\n");
	printf("Option : \n");
	printf(" -i		don't move file to 'trash' directory\n");
	printf(" -r		ask to you when file is removed at on time\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>SIZE [FILENAME] [OPTION]\n");
	printf(" print relative path and file size\n");
	printf("Option : \n");
	printf(" -d NUMBER	print down to NUMBER directory\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>RECOVER [FILENAME] [OPTION]\n");
	printf(" recover files in 'trash' directory to orinigal path\n");
	printf("Option : \n");
	printf(" -l		command after print file and deleted time\n  		in 'trash' directoy(order to upper)\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>TREE\n");
	printf(" show the 'check' directory in tree structure\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>EXIT\n");
	printf(" exit the program\n");
	printf("----------------------------------------------------------\n");
	printf("PROMPT>HELP\n");
	printf(" print usage\n");
	printf("----------------------------------------------------------\n");
}

file_stat* make_tree(char *path) // 디렉토리를 트리화 해주는 함수
{
	int i, count;
	int isFirst = TRUE;
	char tmp[BUFLEN*2];
	file_stat *head = malloc(sizeof(file_stat));
	file_stat *now = malloc(sizeof(file_stat));
	now = head;

	strcpy(head->name, path);
	stat(head->name, &(head->statbuf));

	count = scandir(head->name, &(head->namelist), NULL, alphasort); // 디렉토리에 모든 파일리스트 체크

	for(i = 0;i < count; i++){ // 파일리스트의 모든 파일 분류
		if(!strcmp(head->namelist[i]->d_name, ".") || !strcmp(head->namelist[i]->d_name, "..")) // "."와  ".." 스킵
			continue;

		file_stat *new = malloc(sizeof(file_stat)); // 노드 생성
		new->down = NULL;
		new->next = NULL;

		strcpy(new->name, head->namelist[i]->d_name); // 파일 이름
		sprintf(tmp, "%s/%s", path, new->name); // 절대 경로
		strcpy(new->name, tmp); // 절대 경로로 파일이름 저장

		stat(tmp, &(new->statbuf)); // 해당 파일의 stat 받아옴

		if(S_ISDIR(new->statbuf.st_mode)){ // 디렉토리면
			new = make_tree(tmp); // 다시 트리화
		}

		if(isFirst == TRUE){ // 첫째면 하위로
			now->down = new;
			now = now->down;
			isFirst = FALSE;
		}
		else{ // 그 외의 경우는 형제로 
			now->next = new;
			now = now->next;
		}
	} 
	return head;
}
