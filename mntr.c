#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILELEN 100
#define MAXNUM 100
#define BUFLEN 1024
#define TRUE 1
#define FALSE 0

typedef struct file_status{
	char name[BUFLEN];
	time_t m_time;
	struct dirent **namelist;
	struct stat statbuf;
	struct file_status *down;
	struct file_status *next;
}file_stat;

struct timetable{
	time_t m_time;
	char name[BUFLEN];
};

void ssu_monitoring(char *path);
file_stat* make_tree(char *path);
int count_nodes(file_stat* head);
file_stat* all_nodes(file_stat* head);
file_stat* select_created_node(file_stat *new, file_stat *old);
file_stat* select_deleted_node(file_stat *new, file_stat *old);

int main(void)
{
	FILE* fp;
	DIR *dirp;
	char saved_path[BUFLEN];

	if(access("check", F_OK) < 0) // 모니터링할 디렉토리 없을 시 생성
		mkdir("check", 0755);

	if((fp = fopen("log.txt", "w+")) < 0){ // 모니터링 기록을 남길 파일 생성
		fprintf(stderr, "fopen error for log.txt\n");
		exit(1);
	}

	memset(saved_path, 0, BUFLEN);
	getcwd(saved_path, BUFLEN);
	sprintf(saved_path, "%s/%s", saved_path, "check");
	ssu_monitoring(saved_path);
//	all_nodes(head); // 모든 노드를 출력해주는 함수
	exit(0);

}

void ssu_monitoring(char *path)
{
	int old_count, new_count;
	int isFirst = TRUE;
	struct timetable mod_list[MAXNUM];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

/*printf("now: %d-%d-%d %d:%d:%d\n",
	            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
				         tm.tm_hour, tm.tm_min, tm.tm_sec); */

	file_stat *old_head = malloc(sizeof(file_stat)); // 이전 헤드
	file_stat *new_head = malloc(sizeof(file_stat)); // 새로운 헤드 
	file_stat *tmp = malloc(sizeof(file_stat)); 

	while(1){
	new_head = make_tree(path); // 현재 상태의 트리
	new_count = count_nodes(new_head);

	if(isFirst == TRUE){ // 처음 실행될 경우
		old_head = new_head;
		old_count = count_nodes(old_head);
		isFirst = FALSE;
		continue;
	}

	if(old_count < new_count){ // 파일이 생성된 경우
		if((tmp = select_created_node(new_head, old_head)) == NULL){
			fprintf(stderr, "There is no created node\n");
			exit(1);
		}
		printf("cr>>%s\n", tmp->name);

	}
	else if(old_count > new_count){ // 파일이 삭제된 경우
		if((tmp = select_deleted_node(new_head, old_head)) == NULL){
			fprintf(stderr, "There is no deleted node\n");
			exit(1);
		}
		printf("de>>%s\n", tmp->name);

	}
	else{ // 파일의 개수 변화가 없는 경우

	}

	// 이번 정보를 이전 정보로 변경
	old_head = new_head; 
	old_count = new_count;
	}
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
		else{ // 파일이면
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

int count_nodes(file_stat* head)
{
	int count = 0;
	file_stat *now = malloc(sizeof(file_stat));
	now = head->down;

	while(1){
		count++;
		if(now->down != NULL){
			count += count_nodes(now);
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
	return count;
}

file_stat* select_created_node(file_stat *new, file_stat *old)
{
	file_stat *new_now = malloc(sizeof(file_stat));
	file_stat *old_now = malloc(sizeof(file_stat));
	file_stat *tmp = malloc(sizeof(file_stat));
	new_now = new->down;
	old_now = old->down;

	while(1){
		if(strcmp(new_now->name, old_now->name))
			return new_now;

	    if(new_now->next !=NULL && old_now->next == NULL)
			return new_now->next;

		if(new_now->down != NULL && old_now->down == NULL)
			return new_now->down;

		if(new_now->down != NULL && old_now->down != NULL){
			if((tmp = select_created_node(new_now, old_now)) !=NULL)
				return tmp;
			new_now = new_now->next;
			old_now = old_now->next;
			if(new_now == NULL && old_now == NULL)
				break;
			else if(new_now !=NULL && old_now == NULL)
				return new_now;
		}
		else if(new_now->next != NULL && old_now->next != NULL){
			new_now = new_now->next;
			old_now = old_now->next;
		}
		else
			break;
	}
	return NULL;
}

file_stat* select_deleted_node(file_stat *new, file_stat *old)
{
	file_stat *new_now = malloc(sizeof(file_stat));
	file_stat *old_now = malloc(sizeof(file_stat));
	file_stat *tmp = malloc(sizeof(file_stat));
	new_now = new->down;
	old_now = old->down;

	while(1){
		if(strcmp(new_now->name, old_now->name))
			return old_now;

	    if(new_now->next == NULL && old_now->next != NULL)
			return old_now->next;

		if(new_now->down == NULL && old_now->down != NULL)
			return old_now->down;

		if(new_now->down != NULL && old_now->down != NULL){
			if((tmp = select_deleted_node(new_now, old_now)) != NULL)
				return tmp;
			new_now = new_now->next;
			old_now = old_now->next;
			if(new_now == NULL && old_now == NULL)
				break;
	   		else if(new_now == NULL && old_now != NULL)
				return old_now;
		}
		else if(new_now->next != NULL && old_now->next != NULL){
			new_now = new_now->next;
			old_now = old_now->next;
		}
		else
			break;
	}
	return NULL;
}
