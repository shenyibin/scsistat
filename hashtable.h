#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "readrecords.h"

#define HASH_SIZE 500

struct hash_entry{
	void *p2node;
};

extern struct hash_entry hash_table[HASH_SIZE];

extern void push_hash_entry(char* , struct hash_entry *);

extern int get_hash_entry(char* );

extern void init_hash_table();

extern int DJBHash(char *);

extern void insert_to_hashtable(struct node *);

#endif
