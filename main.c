#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <libperfstat.h>
#include <sys/systemcfg.h>
#include "hashtable.h"
#include "readrecords.h"
#include "statistic.h"

#define PROGRAM "usage"

extern struct record root_curr, root_last;

//for queue
extern int linepool;
extern char gbuf[];

int w_to_file = 0;

#define XINTFRAC     ((double)(_system_configuration.Xint)/(double)(_system_configuration.Xfrac))
#define HTIC2NANOSEC(x,y)  (((double)((x)-(y)) * XINTFRAC)) 

char conf[1024] = {'\0'};
char target[1024] = {'\0'};


void usage()
{
	printf(PROGRAM" is %s -c n -i param...\n \
	\t-c n : the total times to calculate\n \
	\t-f n : config file\n \
	\t-o n : output file\n \
	\t-p n : pool size\n \
	\t-i n : interval to calculate\n", PROGRAM);

	exit(0);
}

int print_each_vg(int file,
		  char *vgname,
		  perfstat_disk_t *curr,
		  perfstat_disk_t *last, 
		  int interval, int nr)
{
	if(curr == NULL || last==NULL) {
		//printf("curr = %x, last =%x\n", curr, last);
		return -1;
	}

	unsigned long long traffic_in, traffic_out;
	float avg_r_io_time,avg_w_io_time;
	float avg_in_size, avg_out_size;
	float r_serv, w_serv, disk_util; 
	unsigned long iops,iops_in,iops_out;
        time_t now = time(0);
        struct tm *ptm = NULL;
        now = time(0);
        ptm = localtime(&now);

	int n;
	traffic_in = (curr->rblks - last->rblks) * 512 / interval; 
	traffic_out = (curr->wblks - last->wblks) * 512 / interval;
	iops = (curr->xfers - last->xfers)  / interval;
	iops_in = (curr->xrate - last->xrate) / interval;
	iops_out = iops - iops_in;
	r_serv = (curr->xrate - last->xrate) ? 
		HTIC2NANOSEC(curr->rserv, last->rserv) * 1.0/1000000/(curr->xrate - last->xrate) : 0;
	w_serv = ((curr->xfers - last->xfers) - (curr->xrate - last->xrate)) ?
		 HTIC2NANOSEC(curr->wserv, last->wserv) * 1.0/1000000/((curr->xfers - last->xfers) - (curr->xrate - last->xrate)) : 0;
	disk_util = (curr->time - last->time) * 1.0 /100 / interval;
	avg_in_size = ((curr->rblks - last->rblks)&& (curr->xrate - last->xrate)) ? 
		(curr->rblks - last->rblks) *512 /  (curr->xrate - last->xrate):0;
	avg_out_size = ((curr->rblks - last->rblks) && ((curr->xfers - last->xfers)-(curr->xrate - last->xrate))) ? 
		(curr->wblks - last->wblks) * 512 / ((curr->xfers - last->xfers)-(curr->xrate - last->xrate)) : 0;
	n = sprintf(gbuf,
		"%04d-%02d-%02d %02d:%02d:%02d," 
		"%s,"
		"%lu,%lu,%lu,%llu,%llu,%.2f,%.2f,"
		"%.2f,%.2f,%.2f\n",
		ptm->tm_year + 1900, ptm->tm_mon + 1,
                ptm->tm_mday, ptm->tm_hour,
                ptm->tm_min, ptm->tm_sec,
		vgname,
		iops, iops_in, iops_out,
		traffic_in, traffic_out,
		avg_in_size,avg_out_size,
		r_serv, w_serv, disk_util);
	gbuf[n]='\0';
	if(file)
		rollback(gbuf);
	else 
		puts(gbuf);
	free(ptm);
	return 0;
}

int print_vginfo(int file,
		 struct record *r1, 
		  struct record *r2,
		  int inval)
{
	struct record *curr, *last;
	curr = r1;
	last = r2;
	while(curr !=NULL && last!= NULL) {
		if (print_each_vg(file, curr->vgname, curr->pstat, last->pstat, inval, curr->disknr) < 0) return -1;
		curr = curr->next;
		last = last->next;
	}
		
}
int main(int argc, char **argv){
	int option;
	int interval;
	int count;
	int i;
	int num_of_rec;			// records the number of records
	perfstat_disk_t *pstat_disk; 
	opterr = 0;
	interval = 5;
	count = -1;
	while((option = getopt(argc, argv, "i:c:f:o:p:h")) != -1){
		switch(option){
		case 'i':
			interval = atoi(optarg);
			break;
		case 'c':
			count = atoi(optarg);
			break;
		case 'f':
			strncpy(conf, optarg, strlen(optarg));
			break;
		case 'o':
			strncpy(target, optarg, strlen(optarg));
			w_to_file = 1;
			break;
		case 'p':
			linepool = atoi(optarg)	;
			break;
		case 'h':
		case '?':
			usage();
		}
	}
	// read configs from conf file
	init_hash_table();
	init_records();
	num_of_rec = perfstat_disk(NULL, NULL, sizeof(perfstat_disk_t), 0);
	pstat_disk = (perfstat_disk_t *)malloc(num_of_rec*sizeof(perfstat_disk_t));
	if(pstat_disk == NULL){
		perror("pstat_disk malloc fail");
		return -1;
	}
	perfstat_id_t first_disk = {.name = FIRST_DISK};
	i = 0;
	if(!w_to_file)
		fprintf(stdout, "TIME,VGNAME,IOPS,RIOPS,WIOPS,RTRAFFIC,WTRAFFIC,RSIZE,WSIZE,RSERV,WSERV,UTIL\n");
	while(i < count || count < 0){
		clear_the_record_list(&root_curr);
		if(collect_disk_info(pstat_disk, num_of_rec) < 0) {
			perror("perfstat disk error");
			return -1;
		}
		merge_disk(pstat_disk, num_of_rec);
		print_vginfo(w_to_file, &root_curr, &root_last, interval);
		rotate_record_list(&root_curr, &root_last);
		sleep(interval);
		++i;
	}
	//FIXme:
	free(pstat_disk);
	//free_records();
	return 0;
}
