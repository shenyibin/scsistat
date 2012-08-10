#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libperfstat.h>
#include "readrecords.h"
#include "hashtable.h"

struct record  root_curr = {NULL, NULL, NULL}, root_last = {NULL, NULL, NULL};

extern char  conf[1024];

perfstat_disk_t * searchvg(struct record *root, char *n)
{
	perfstat_disk_t *p;
	struct record *tmp;
	tmp = root;
	p = NULL;
	while(tmp != NULL) {
		if (!strcmp(tmp->vgname, n)) {
			p = tmp->pstat;
			break;
		}
		tmp = tmp->next;
	}
	//printf("searched address = %x\n", p);
	return p;
}
void clear_the_record_list(struct record *root) {
	struct record *tmp;
	tmp = root;
	while(tmp!=NULL) {
		memset(tmp->pstat, '0', sizeof(perfstat_disk_t));
		tmp = tmp->next;
	}
	return;
}
void insert_a_record(struct record *root, struct record *newrecord)
{
	struct record *tmp;
	tmp = root;
	//root node maybe not used yet
	if(tmp->pstat == NULL) {
		tmp->pstat = newrecord->pstat;
		tmp->vgname = newrecord->vgname;
		tmp->disknr = newrecord->disknr;
		return;
	}
	while(tmp->next != NULL)
		tmp = tmp->next;
	tmp->next = newrecord;
}

int init_records(){
	FILE * fp;
	char buffer[LINE_OF_CHAR];
	char *token;
	char *vgname;
	int loop;

	loop = 0;
	
	if (conf[0] == '\0') //unset config file, use default
		strcpy(conf, CONFIG_FILE);
	
	if((fp = fopen(conf, "r")) == NULL){
		perror("can't open the conf file");
		return -1;
	}

	while(fgets(buffer, LINE_OF_CHAR, fp) != NULL){
		if(buffer[strlen(buffer)-1] == '\n'){
			buffer[strlen(buffer)-1] = 0;
		}
		token = strtok(buffer, "=");
		if(token == NULL)
			continue;
		
		vgname = strdup(token);
		struct record *tmp;
		tmp = (struct record *) malloc(sizeof(struct record));
		tmp->vgname=vgname;
		tmp->pstat = (perfstat_disk_t *)malloc(sizeof(perfstat_disk_t));
		tmp->next = NULL;
		//get the record name;
		while((token = strtok(NULL, ",")) != NULL){
			struct node *tmpnode;
			tmpnode = (struct node * )malloc(sizeof(struct node));
			if(tmpnode==NULL) {
				perror("error while allocate memory");
				return -1;
			}
			strcpy(tmpnode->diskname, token);
			tmpnode->vgname[strlen(token)+1] = '\0';
			strcpy(tmpnode->vgname, vgname);
			tmpnode->vgname[strlen(vgname)+1] = '\0';
			tmpnode->next = NULL;
			insert_to_hashtable(tmpnode);
			//increase the number of disks in this group
			tmp->disknr++;
		}
		insert_a_record(&root_curr, tmp);
		++loop;
		if(loop == 100){
			break;
		}
	}
	struct record *record_tmp;
	record_tmp = & root_curr;
	return 0;
}

void free_list(struct record *root)
{
	if(root->next == NULL) {
		if(root->vgname) 
			free(root->vgname);
		if(root->pstat) 
			free(root->pstat);
		free(root);
	}
	free_list(root->next);
}

void free_records()
{
	//ignore the head of the list
	free_list(root_curr.next);
	free_list(root_last.next);
}
