#ifndef _READRECORDS_H
#define _READRECORDS_H

#include <libperfstat.h>

#define CONFIG_FILE "records.conf"
#define NUM_OF_RECORDS 1000
#define LINE_OF_CHAR 40960
#define MAX_NAME_LEN 128

struct node {
	char vgname[MAX_NAME_LEN];
	char diskname[MAX_NAME_LEN];
	struct node * next;
};

struct record {
	char *vgname;
	perfstat_disk_t *pstat;
	int disknr;
	struct record *next;
};

extern void insert_a_record(struct record *, struct record *);
extern perfstat_disk_t * searchvg(struct record *, char *);
extern int init_records();
extern void free_records();
extern void clear_the_record_list(struct record *); 
#endif
