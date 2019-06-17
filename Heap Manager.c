/*
Author: Jonathan Reed
Instructor: Dr. Butler
Course: CSCI 3240
Year: 2018 (spring)
Purpose: Write a collection of C functions that provide the functionality of a heap manager
* Note  - June 2019: There are some optimizations I've learned since writing this. For instance, 
*  the ordering of the node struct is inefficient - data members ought to instead
*  be arranged in descending order of size. I've left it unchanged.
*/
//includes
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//typedefs

typedef struct node
{
	int size;
	void *heap_area; 
	struct node *next;
	bool free;
}node;

//variables
static int total_bytes;
static node *heap_start;
static node *heap_current;
static node *heap_next;
static node *heap_max_addr;
static node *head;

//functions
node *create_node(int size, void *heap_area, node *next, bool free);
void append_node(node *new_node, node *head);
void insert_node(node *new_node, node *prev_node, node *head);
void coalesce (void *area_ptr);

void  heap_init(int num_pages)
{
	total_bytes = getpagesize()*num_pages;	
	heap_start = mmap(NULL, total_bytes, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	heap_max_addr = heap_start + (total_bytes/32);
	heap_current = heap_start; 
	head = create_node(0, heap_current, NULL, 0);
	return;
}

void *heap_alloc(int num_bytes)
{
	int remainder = num_bytes % 16;
	if (remainder != 0)
	{
		num_bytes += 16-remainder;
	}
	//3 cases:
		//1: list is empty
		//2: insert at back of list
		//3: insert in middle of list
	node *previt = NULL;
	node *itr = head;
	node *heap_it = heap_start;
	node *temp_node = create_node(num_bytes, heap_current, NULL, 0);
	//case 1:
	if (itr -> size == 0)
	{
		head = temp_node;
		heap_current += (temp_node -> size / 32);
		return heap_it;
	}
	if ((heap_current + (num_bytes/32)) > heap_max_addr)
	{
		//case 3:
		node *itr = head;
		previt = NULL;
		while (itr -> next != NULL)
		{
			previt = itr;
			itr = itr -> next;
			if ( (itr -> heap_area - (previt -> heap_area + previt -> size) ) > num_bytes )
			{
				void *new_node_area = (previt -> heap_area) + (previt -> size);
				node *temp_node = create_node(num_bytes, new_node_area, NULL, 0);
				temp_node -> next = itr;
				previt -> next = temp_node;
				return temp_node -> heap_area;
			}
		}
		return NULL;
	}
	heap_current += (num_bytes/32);
	while (itr -> next != NULL)
	{
		previt = itr;
		itr = itr -> next;
		if (itr -> free == 1)
		{
			if (itr -> size = num_bytes)
			{
				itr = temp_node;
				return itr;
			}
		}
		heap_it += (itr -> size / 32);
	}
	itr -> next = temp_node;
	return temp_node -> heap_area;
}

void heap_free(void *area_ptr)
{
	node *previt = NULL;
	node *itr = head;
	while (itr -> heap_area != area_ptr && itr -> next != NULL)
	{
		previt = itr;
		itr = itr -> next;	
	}
	itr -> free = 1;
	coalesce(itr);
}

void coalesce (void *area_ptr)
{
	node *previt = NULL;
	node *itr = head;
	bool match = 0;
	
	while (match == 0 && itr -> next != NULL)
	{
		previt = itr;
		itr = itr -> next;
		if (itr == area_ptr)
			match = 1;
	}
	if (previt -> free == 1)
	{
		previt -> next = itr -> next;
		itr -> next = NULL;
	}
	previt -> next = itr -> next;
	itr -> next = NULL;
	free(itr);
}

node *create_node(int size, void *heap_area, node *next, bool free)
{
	node *new_node = malloc(sizeof(node));
	new_node -> size = size;
	new_node -> heap_area = heap_area;
	new_node -> next = next;
	new_node -> free = free;
}

void append_node(node *new_node, node *head)
{
	node *itr = head;
	while (itr -> next != NULL)
	{
		itr = itr -> next;
	} 
	itr -> next = new_node;
}

void insert_node(node *new_node, node *prev_node, node *head)
{
	node *previt = NULL;
	node *itr = head;
	
	while (itr -> next != NULL)
	{
		previt = itr;
		itr = itr -> next;
		if (previt == prev_node) //insert here
		{
			new_node -> next = itr;
			previt -> next = new_node;
		}
	}	
}

