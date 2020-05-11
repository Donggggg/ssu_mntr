#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ssu_mntr.h"

char saved_path[BUFLEN], check_path[BUFLEN+6], log_path[BUFLEN+8];
struct timetable change_list[MAXNUM];

int main(void)
{
	FILE* fp;
	DIR *dirp;

	memset(saved_path, 0, BUFLEN);
	getcwd(saved_path, BUFLEN); // 현재 작업 경로를 저장
	sprintf(check_path, "%s/%s", saved_path, "check"); // 모니터링할 디렉토리의 경로
	sprintf(log_path, "%s/%s", saved_path, "log.txt"); // 로그파일의 경로 

	if(access(check_path, F_OK) < 0) // 모니터링할 디렉토리 없을 시 생성
		mkdir(check_path, 0755);

	if((fp = fopen(log_path, "w+")) < 0){ // 모니터링 기록을 남길 파일 생성
		fprintf(stderr, "fopen error for log.txt\n");
		exit(1);
	}
	fclose(fp);

	daemon_init(); // 디몬 프로세스로 변경
	ssu_monitoring(check_path); // 모니터링 시작

	exit(0);
}

int daemon_init(void)
{
	pid_t pid;
	int fd, maxfd;

	if((pid = fork()) < 0){
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid != 0){ // 부모 종료
		exit(0);
	}

	setsid(); // 새로운 프로세스 그룹 생성
	// 표준 입출력, 제어 무시
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	maxfd = getdtablesize();

	for(fd = 0; fd < maxfd; fd++) // 모든 파일 디스크립터 닫음
		close(fd);

	umask(0); // 파일 모드 마스크 해제
	chdir("/"); // 루트 디렉토리로 이동
	fd = open("dev/null", O_RDWR); // 표준입출력 재지정
	dup(0);
	dup(0);
	return 0;
}

void ssu_monitoring(char *path)
{
	int old_count, new_count;
	int count;
	int isFirst = TRUE;
	time_t t = time(NULL);

	file_stat *old_head = malloc(sizeof(file_stat)); // 이전 헤드
	file_stat *new_head = malloc(sizeof(file_stat)); // 새로운 헤드 
	file_stat *tmp = malloc(sizeof(file_stat)); 

	while(1){
		new_head = make_tree(path); // 현재 상태의 트리
		new_count = count_nodes(new_head); // 현재 파일의 총 개수 
		init_node_content(new_head->down, UNCHECKED);

		if(isFirst == TRUE){ // 처음 실행될 경우
			old_head = new_head;
			old_count = count_nodes(old_head);
			isFirst = FALSE;
			continue;
		}

		compare_tree(new_head->down, old_head->down); // 두 트리 비교
		count = set_list(new_head->down, 0, CREATE); // CREATE된 파일 체크
		count = set_list(old_head->down, count, DELETE); // DELETE된 파일 체크
		sort_time_table(count); // 시간 순에 맞춰 리스트 정렬

		write_log(count); // "log.txt"에 변경사항 입력

		// 이번 정보를 이전 정보로 변경
		old_head = new_head; 
		old_count = new_count; 
		init_node_content(old_head->down, UNCHECKED);

		sleep(1); // 딜레이 
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

void init_node_content(file_stat *node, int status)
{
	file_stat *now = malloc(sizeof(file_stat));
	now = node;

	if(now == NULL)
		return ;

	while(1)
	{
		now->content = status; // 해당 노드 상태 변경

		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->down != NULL)
				init_node_content(now->down, status);
		}

		if(now->next != NULL)
			now = now->next;
		else
			break; 
	} 
}

int count_nodes(file_stat* head)
{
	int count = 0;
	file_stat *now = malloc(sizeof(file_stat));
	now = head->down;

	if(now == NULL)
		return 0;

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

void compare_tree(file_stat *new, file_stat *old)
{
	file_stat *now = malloc(sizeof(file_stat));
	now = old;

	if(new == NULL || old == NULL)
		return ;

	while(1)
	{
		compare_node(new, now);

		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->down != NULL)
				compare_tree(new, now->down);
		}

		if(now->next != NULL)
			now = now->next;
		else
			break;
	}
}

int compare_node(file_stat *new, file_stat *old)
{
	file_stat *now = malloc(sizeof(file_stat));
	now = new;

	while(1)
	{
		if(!strcmp(now->name, old->name)){ // 해당노드가 존재할 경우
			now->content = CHECKED;
			if(now->statbuf.st_mtime != old->statbuf.st_mtime) // 해당노드의 수정시간이 변경된 경우
				now->content = MODIFY;
			old->content = CHECKED;
			return -1;
		}

		if(S_ISDIR(now->statbuf.st_mode))
			if(now->down != NULL)
				if(compare_node(now->down, old) < 0)
					break;

		if(now->next != NULL)
			now = now->next;
		else
			break;
	}
}

int set_list(file_stat *node, int count, int status)
{
	file_stat *now = malloc(sizeof(file_stat));
	now = node;

	if(node == NULL)
		return count;

	while(1)
	{
		switch(now->content){
			case UNCHECKED : // 생성 or 삭제
				change_list[count].m_time = time(NULL);
				strcpy(change_list[count].name, now->name);
				change_list[count++].content = status;
				break;
			case MODIFY : // 수정
				change_list[count].m_time = now->statbuf.st_mtime;
				strcpy(change_list[count].name, now->name);
				change_list[count++].content = MODIFY;
				break;
		}

		if(S_ISDIR(now->statbuf.st_mode)){
			if(now->down != NULL)
				count = set_list(now->down, count, status);
		}

		if(now->next != NULL)
			now = now->next;
		else
			break; 
	} 
	return count;
}

void write_log(int count)
{
	int i;
	char *tmp, filename[BUFLEN];
	char fullname[BUFLEN+8];
	char time_format[TIMEFORM];
	FILE *fp;
	struct tm tm;

	if((fp = fopen(log_path, "r+")) < 0){
		fprintf(stderr, "fopen error for log.txt\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);

	for(i=0; i<count; i++){
		tmp = strstr(change_list[i].name, "check/");
		tmp += 6;
		strcpy(filename, tmp);
		while((tmp =strchr(filename, '/')) != NULL)
			*tmp = '_';

		switch (change_list[i].content){
			case CREATE :
				tm = *localtime(&change_list[i].m_time);
				sprintf(time_format, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
						tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec); 
				sprintf(fullname, "%s_%s", "create", filename);
				fprintf(fp, "[%s][%s]\n", time_format, fullname);
				break;
			case DELETE :
				tm = *localtime(&change_list[i].m_time);
				sprintf(time_format, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
						tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec); 
				sprintf(fullname, "%s_%s", "delete", filename);
				fprintf(fp, "[%s][%s]\n", time_format, fullname);
				break;
			case MODIFY :
				tm = *localtime(&change_list[i].m_time);
				sprintf(time_format, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
						tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec); 
				sprintf(fullname, "%s_%s", "modify", filename);
				fprintf(fp, "[%s][%s]\n", time_format, fullname);
				break;
		}
	}
	fclose(fp);
}

void sort_time_table(int max)
{
	int i,j;
	struct timetable tmp;

	for(i = 0; i < max; i++)
		for(j = i+1; j < max; j++){
			if(change_list[i].m_time > change_list[j].m_time){
				tmp = change_list[i];
				change_list[i] = change_list[j];
				change_list[j] = tmp;
			}
		}
}
