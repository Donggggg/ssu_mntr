#define FILELEN 100
#define MAXNUM 100
#define BUFLEN 1024
#define TIMEFORM 20
#define CREATE 2
#define DELETE 1
#define MODIFY 0
#define CHECKED -1
#define UNCHECKED -2
#define TRUE 1
#define FALSE 0

typedef struct file_status{
	char name[BUFLEN];
	int content;
	struct dirent **namelist;
	struct stat statbuf;
	struct file_status *down;
	struct file_status *next;
}file_stat;

struct timetable{
	time_t m_time;
	char name[BUFLEN];
	int content;
};

void ssu_monitoring(char *path);
int count_nodes(file_stat* head);

file_stat* make_tree(char *path);
void compare_tree(file_stat *new, file_stat *old);
int compare_node(file_stat *new, file_stat *old);

void init_node_content(file_stat *node, int status);
int set_list(file_stat *node, int count, int status);
void write_log(int count);
void sort_time_table(int max);
file_stat* all_nodes(file_stat* head);
