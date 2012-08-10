#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "hashtable.h"

struct hash_entry hash_table[HASH_SIZE];

void insert_to_hashtable(struct node *n)
{
	int key;
	struct node *node;
	
	//generate the hash key
	key = DJBHash(n->diskname);
	//get hash entry
	node = hash_table[key].p2node;
	if(node == NULL) {
		hash_table[key].p2node = n;
	} 
	else {
		//go to tail
		while(node->next != NULL) {
			node = node->next;
		}
		//add new node to tail;
		node->next = n;
	}
}

int  get_hash_entry(char* hash_key){
	return DJBHash(hash_key);
}

void init_hash_table()
{
	memset(hash_table, 0, sizeof(hash_table));
}

int DJBHash(char* str)
{
	int hash = 5381;
	int i;
	for(i = 0; i < strlen(str); i++)
		{
			hash = ((hash << 5) + hash) + str[i];
		}

	return (hash & 0x7FFFFFFF) % HASH_SIZE;
}
