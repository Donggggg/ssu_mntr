#define BUFLEN 1024
#define FILELEN 100
#define MAXNUM 100
#define TIMEFORM 20

#define CREATE 2
#define DELETE 1
#define MODIFY 0

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
	int content;
};

void ssu_monitoring(char *path);
file_stat* make_tree(char *path);
int count_nodes(file_stat* head);
file_stat* all_nodes(file_stat* head);
file_stat* select_created_node(file_stat *new, file_stat *old);
file_stat* select_deleted_node(file_stat *new, file_stat *old);
file_stat* select_modified_node(file_stat *new, file_stat *old);
file_stat* find_node(file_stat *check, file_stat* head);
void write_log(struct timetable *mod_list, int count);



