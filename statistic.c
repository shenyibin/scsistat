#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>	

#include <libperfstat.h>
#include "hashtable.h"
#include "statistic.h"
#include "readrecords.h"

extern struct record root_curr, root_last;

int collect_disk_info(perfstat_disk_t * pstat, int nr)
{
	//clear the buffer
	memset(pstat, '0', nr * sizeof(perfstat_disk_t));
	perfstat_id_t first_disk = {.name = FIRST_DISK};
	return perfstat_disk(&first_disk, pstat, sizeof(perfstat_disk_t), nr);
}

void copy_list(struct record *from, struct record * to )
{
	struct record *tmp;
	while(from != NULL) {
		//generate a record
		tmp = (struct record *) malloc(sizeof(struct record));
		tmp->vgname=from->vgname;
		tmp->pstat = (perfstat_disk_t *)malloc(sizeof(perfstat_disk_t));
		memcpy(tmp->pstat, from->pstat, sizeof(perfstat_disk_t));
		tmp->next = NULL;
		//
		insert_a_record(to, tmp);
		from = from->next;
	}
	return;
}

void rotate_record_list(struct record *from, struct record *to)
{
	struct record * curr, * last;
	curr = from ; last = to;
	if (last->pstat == NULL) {
		copy_list(curr, last);
		return;
	}
	while(curr !=NULL && last != NULL) {
		memcpy(last->pstat, curr->pstat, sizeof(perfstat_disk_t));
		curr=curr->next;
		last=last->next;
	}
}

void plus_one_disk(perfstat_disk_t *vg, perfstat_disk_t *disk)
{
	vg->xfers += disk->xfers;
	vg->time += disk->time;
	vg->qdepth += disk->qdepth;
	vg->q_sampled += disk->q_sampled;
	vg->wq_sampled += disk->wq_sampled;
	vg->wq_depth += disk->wq_depth;
	vg->q_full += disk->q_full;
	vg->xrate += disk->xrate;
	vg->rblks += disk->rblks;
	vg->rserv += disk->rserv;
	vg->rtimeout += disk->rtimeout;
	vg->max_rserv += disk->max_rserv;
	vg->wblks += disk->wblks;
	vg->wserv += disk->wserv;
	vg->wtimeout += disk->wtimeout;
	vg->wfailed += disk->wfailed;
	vg->min_wserv += disk->min_wserv;
	vg->max_wserv += disk->max_wserv;
}


void mergeit(char *vgname, perfstat_disk_t *pstat)
{
	perfstat_disk_t *thisvg;
	//printf("vgname = %s\n", vgname);
	thisvg = searchvg(&root_curr, vgname);
	if (thisvg == NULL) {
		perror("BUG");
		exit(1);
	}
	plus_one_disk(thisvg, pstat);
	return;
}

void merge_disk(perfstat_disk_t *pstat, int nr)
{
	int i, key;
	for(i=0; i<nr; i++) {
		key = get_hash_entry((pstat+i)->name);
		//ignore the trial disk
		if (hash_table[key].p2node == NULL) continue;
		struct node* node;
		node = hash_table[key].p2node;
		while(node!=NULL) {
			//find the node 
			if(!strcmp((pstat+i)->name, node->diskname)) {
				mergeit(node->vgname, pstat+i);
				break;
			}
			node=node->next;
		}
	}
}
