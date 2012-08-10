#ifndef _STATISTIC_H
#define _STATISTIC_H

#include <sys/types.h>

#define NUM_OF_STATIS 100

#define u_longlong_t unsigned long long

struct statis{
	u_longlong_t xfers;
	u_longlong_t time;
	u_longlong_t qdepth;
	u_longlong_t  q_sampled;
	u_longlong_t xrate;
	u_longlong_t rblks;
	u_longlong_t  rtimeout;
	u_longlong_t  max_rserv; 
	u_longlong_t wblks;
	u_longlong_t  wserv;
	u_longlong_t  wtimeout;
	u_longlong_t  wfailed;
	u_longlong_t  min_wserv;
	u_longlong_t  max_wserv;
} ;

extern void print_statis();
extern void calc_statis();
extern int collect_disk_info(perfstat_disk_t * , int );
extern void merge_disk(perfstat_disk_t *, int);
extern void rotate_record_list(struct record *, struct record *);

#define FIRST_DISK ""

#endif
