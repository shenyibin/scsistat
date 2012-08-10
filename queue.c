#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "queue.h"

struct line{
        char text_line[MAX_RECORD_LEN];
        struct line * next;
};

typedef struct line * LINE;
static LINE head;
static int num_of_line = 0;
int linepool = 1000;
char gbuf[MAX_RECORD_LEN] = {'\0'};
static int inited = FALSE;
extern char target[];
int push(char *text)
{
        LINE p,q;

        ++num_of_line;

        if(num_of_line > linepool){
                p  = head;
                head = head->next;
                free(p);
                --num_of_line;
        }

        p = head;
        q = (LINE)malloc(sizeof(struct line));
        if(q == NULL){
                printf("malloc fails\n");
                return 0;
        }
        strncpy(q->text_line, text, MAX_RECORD_LEN);
        q->next = NULL;

        if(p == NULL){
                head = q;
                return 1;
        }

        while(p->next != NULL)
                p = p->next;
        p->next = q;
        return 1;
}


int init_list()
{
	FILE *fp;
        char buf[MAX_RECORD_LEN];
	fp = fopen(target, "r+");
	if(fp != NULL)
		while(fgets(buf, MAX_RECORD_LEN,fp) != NULL){
			push(buf);
		}
	else {
		push(" ");
		inited = TRUE;
	}
	fflush(fp);
	fclose(fp);
        return 1;
}

void free_queue_list()
{
        LINE p, q;

        p = head;
        while(p != NULL){
                q = p;
                p = p->next;
                free(q);
        }

        head = NULL;
}

void writeout()
{
        LINE p;
	FILE *fp;
        p = head;
 	fp = fopen(target, "w+");
        while(p->next != NULL){
                fputs(p->text_line, fp);
                p = p->next;
        }
        fputs(p->text_line,fp);
	fflush(fp);
	fclose(fp);
}

void rollback(char *line)
{
	if(head == NULL) {
		init_list();
	}
	push(line);
	writeout();
}
