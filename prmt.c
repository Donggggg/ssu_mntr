#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mntr.h"

//#define BUFLEN 1024
//#define MAXNUM 100
void delete_file_on_time(int sec, char *path, char *filename);
void print_size(file_stat *node, char *path, int d_num, int print_all);
void print_tree(file_stat *node, int level, int *length, int *check);
struct tm* get_time(char * time_string);
int get_time_diffrence(struct tm tm);
void to_lower_case(char *str);
void print_usage();

file_stat size_table[BUFLEN];

int main(void)
{
	int i,j;
	int count, d_option; 
	int	direc_length[MAXNUM],lastfile_check[MAXNUM];
	char command_line[BUFLEN], path[BUFLEN], main_path[BUFLEN], *apath, *rpath;
	char *dpath;
	char command_tokens[BUFLEN][MAXNUM], time_table[4][5];
	char *command, *tmp;
	char filename[FILELEN], direcname[FILELEN], direc_path[BUFLEN];
	struct dirent **namelist;
	struct tm *tm;
	time_t now, reserv;	
	int diff;
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
		memset(main_path, 0, BUFLEN);
		getcwd(main_path, BUFLEN);
		sprintf(path, "%s/%s", main_path, "check");

		if(!strcmp(command, "delete"))
		{
			if(!strcmp(command_tokens[0], "")){ // FILENAME 값이 없으면 예외처리
				printf("Input the [FILENAME] please..\n");
				continue;
			}

			dpath = malloc(sizeof(char) * BUFLEN);
			realpath(command_tokens[0], dpath); // 삭제할 파일의 경로(절대경로)
			printf("ab path >%s\n", dpath);

			strcpy(filename, command_tokens[0]);
			tmp = strtok(command_tokens[0], "/"); // 삭제할 파일명 추출
			while((tmp = strtok(NULL, "/")) != NULL)
				strcpy(filename, tmp);
			printf("delfile> %s\n", filename);

			tmp = dpath;
			count = 0;
			for(tmp += strlen(dpath)-strlen(filename)-2; *tmp != '/'; tmp--)
				count++;
			strncpy(direcname, tmp+1, count); // 삭제할 파일의 디렉토리명 추출
			printf("direc> %s\n", direcname);
			memset(direc_path, 0, BUFLEN);
			strncpy(direc_path, dpath, strlen(dpath) - strlen(filename)-1); // 삭제할 파일의 디렉토리의 절대경로

			count = scandir(direc_path, &namelist, NULL, alphasort);
			for(i = 0; i < count; i++){ // 삭제할 파일이 해당 디렉토리에 있는지 검사
				if(!strcmp(namelist[i]->d_name, filename))
					break;
			}
			if(i==count){ // 없으면 예외처리
				printf("file(%s) is not in directory(%s)\n", filename, direcname);
				continue;
			}

			if(access("trash", F_OK) < 0) // trash폴더 생성
				mkdir("trash", 0755);

			tmp = malloc(sizeof(char) * BUFLEN);
			strcpy(tmp, "./trash");
			chdir(tmp); // trash디렉토리로 작업 이동
			mkdir("files", 0755); // files폴더 생성
			mkdir("infos", 0755); // infos폴더 생성
			chdir(main_path); // 다시 메인디렉토리로 이동

			sprintf(command_tokens[1], "%s %s ", command_tokens[1], command_tokens[2]);
			tm = get_time(command_tokens[1]); // 삭제 예약시간에 대한 정보를 저장

			if(tm->tm_sec < 0){ // 입력 오류 예외 처리
				printf("input wrong time form\n");
				continue;
			}

			now = time(NULL); // 현재시간
			reserv = mktime(tm); // 삭제가 예약된 시간
			diff = difftime(reserv, now); // 두 시간의 차 구함
			printf("wait time : %d\n", diff);

			free(tmp);
		}
		else if(!strcmp(command, "size"))
		{
			head = make_tree(path);
			d_option = 1;
			strcpy(filename, command_tokens[0]);
			apath = malloc(sizeof(char) * BUFLEN);
			getcwd(apath, BUFLEN);

			if(filename[0] != '.') // 절대경로로 변경
				sprintf(apath, "%s/%s", apath, filename);
			else
				realpath(filename, apath);

			if(!strcmp(command_tokens[1], "-d"))
				if((d_option = atoi(command_tokens[2])) == 0){
					printf("Wrong input at option NUMBER\n");
					continue;
				}

			print_size(head, apath, d_option, FALSE);
			free(apath);
		}
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

struct tm * get_time(char * time_string)
{
	int i, pos = 0, level = 0;
	char str[4];
	struct tm *tm = malloc(sizeof(struct tm));
	tm->tm_sec = 0;

	for(i = 0; i < strlen(time_string); i++){
		if(time_string[i] <='9' && time_string[i] >= '0')
			str[pos++] = time_string[i];
		else if(time_string[i] == '-' || time_string[i] == ' ' || time_string[i] == ':'){
			if(level == 0) // 년도
				tm->tm_year = atoi(str) - 1900;
			else if(level == 1) // 월
				tm->tm_mon = atoi(str) - 1;
			else if(level == 2) // 일
				tm->tm_mday = atoi(str);
			else if(level == 3) // 시
				tm->tm_hour = atoi(str);
			else if(level == 4) // 분
				tm->tm_min = atoi(str);
			pos = 0;
			str[2] = '\0';
			level++;
		}
		else
			tm->tm_sec = -1; // 예외처리
	}
	return tm;
}

void delete_file_on_time(int sec, char *path, char *filename)
{

}

void print_size(file_stat *node, char *path, int d_num, int print_all)
{
	char cur_path[BUFLEN], *rpath;
	int isTrue = print_all;
	file_stat *now = malloc(sizeof(file_stat));
	now = node;

	while(d_num != 0)
	{
		if(print_all == TRUE){
			memset(cur_path, 0, BUFLEN);
			getcwd(cur_path, BUFLEN);
			rpath = now->name + strlen(cur_path);	
			printf("%ld	.%s\n", now->statbuf.st_size, rpath);
		}
		else if(!strcmp(now->name, path)){
			if(d_num == 1){
				d_num--;
				memset(cur_path, 0, BUFLEN);
				getcwd(cur_path, BUFLEN);
				rpath = now->name + strlen(cur_path);	
				printf("%ld	.%s\n", now->statbuf.st_size, rpath);
				break;
			}
			else if(d_num > 1){
				if(!S_ISDIR(now->statbuf.st_mode)){
					printf("-d option is only for directory, not file\n");
					break;
				}
				isTrue = TRUE;
			}
		}

		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->down != NULL){
				if(isTrue == TRUE)
					print_size(now->down, path, d_num-1, TRUE);
				else if(isTrue == FALSE)
					print_size(now->down, path, d_num, FALSE);
			}
		}

		if(now->next != NULL)
			now = now->next;
		else
			break; 
	} 
}

void print_tree(file_stat *node, int level, int *length, int *check)
{
	int i, j, isFirst=TRUE, max;
	char *filename, *tmp;
	file_stat *now = malloc(sizeof(file_stat));
	now = node;
	check[level-1] = FALSE;

	while(1)
	{
		filename = strtok(now->name, "/"); // 파일명만 추출
		while((tmp = strtok(NULL, "/")) != NULL)
			strcpy(filename, tmp);

		max = 6;
		if(isFirst == TRUE){ // 디렉토리 하위파일의 처음인 경우
			printf("|");
			isFirst = FALSE;
		}
		else{
			for(i=0; i<level; i++){ // 디렉토리의 깊이만큼
				for(j = 0; j < length[i]; j++) // 디렉토리명의 길이만큼 공백
					printf(" ");
				if(i != 0) // 1단계이후부터 max를 7로
					max = 7;
				for(j = 0; j < max; j++)
					printf(" ");
				if(check[i] == FALSE) // 디렉토리내의 파일을 이어주는 표시
					printf("|");
				else 
					printf(" ");
			}
		}

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
