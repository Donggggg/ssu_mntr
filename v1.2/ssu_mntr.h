#include <sys/stat.h>

#define BUFLEN 1024
#define FILELEN 100
#define MAXNUM 100
#define TIMEFORM 20

#define CREATE 2
#define DELETE 1
#define MODIFY 0
#define CHECKED -1
#define UNCHECKED -2

#define TRUE 1
#define FALSE 0

#define MAXSIZE 2000

/* 파일 노드 구조체 */
typedef struct file_status{
	char name[BUFLEN];
	int content;
	struct dirent **namelist;
	struct stat statbuf;
	struct file_status *down;
	struct file_status *next;
}file_stat;

/* 파일 수정시간 테이블 */
struct timetable{
	time_t m_time;
	char name[BUFLEN];
	int content;
};

/*** ssu_mntr(모니터링) 관련 함수 원형 ***/
int daemon_init(void);
void ssu_monitoring(char *path);

file_stat* make_tree(char *path);
void compare_tree(file_stat *new, file_stat *old);
int compare_node(file_stat *new, file_stat *old);
int count_nodes(file_stat* head);

void init_node_content(file_stat *node, int status);
int set_list(file_stat *node, int count, int status);
void write_log(int count);
void sort_time_table(int max);
file_stat* all_nodes(file_stat* head);

/*** main(프롬프트) 관련 함수 원형 ***/
void ssu_prompt(void);

void delete_file_on_time(int sec, char *path, char *filename);
void delete_file(char *saved_path, char *path, char *filename);
void remove_file_on_time(int sec, char *path);
int make_unoverlap_name(struct dirent **namelist, int count, char* filename, int cur);
void check_infos();
void optimize_trash(struct dirent **namelist, int count);

void recover_file(char *filename);
void sort_files_by_mtime(struct dirent **namelist, int count);

void print_size(file_stat *node, char *path, int d_num, int print_all);
long int print_sum_of_down_files(file_stat *node);

void print_tree(file_stat *node, int level, int *length, int *check);

struct tm* get_time(char * time_string);
void to_lower_case(char *str);
void print_usage();
