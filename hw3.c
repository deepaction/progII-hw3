#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define ARGUMENT_NUMBER 20

struct list
{
	pid_t pid;
	char *args[ARGUMENT_NUMBER];
	int run;
	struct list *next;
} *head;

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}

void list_init()
{
	head=(struct list *)malloc(sizeof(struct list));
	if(head==NULL)
	{
		fprintf(stderr,"%s: Could not allocate dynamic memory (Error in line: %d)\n", __FILE__, __LINE__-3);
		exit(-1);
	} 	
	
	head->next=head;
}

struct list *create_list_entry(pid_t pid, char *args[ARGUMENT_NUMBER], int run) 
{
	
	int i;
	struct list *curr;
	
	curr=(struct list *)malloc(sizeof(struct list));
	if(curr==NULL)
	{
		fprintf(stderr,"%s: Could not allocate dynamic memory (Error in line: %d)\n", __FILE__, __LINE__-3);
		exit(-1);
	}
	
	curr->pid=pid;
	for(i=0; i<ARGUMENT_NUMBER; i++)
	{
		if(args[i]==NULL)
			break;
		curr->args[i]=strdup(args[i]);
		
	}
	curr->args[i]=NULL;
	curr->run=run;
	
	return(curr);
}

void list_insert(struct list *curr)
{
	struct list *temp = head;
	
	if(head->next == head) 
	{
		head->next = curr;
		curr->next = head;
	}
	else
    {
		while(1) 
		{ 
			if(temp->next == head)
			{
				temp->next=curr;
				curr->next=head;
				break;
			}
			temp=temp->next;
		}
	}
}

void list_delete(pid_t pid)
{
	int i;
	struct list *freenode;
	struct list *temp = head;
	while(temp->next!=head)
	{
		if (temp->next->pid == pid) 
		{
			freenode=temp->next;
			temp->next=temp->next->next;
			for(i=0; i<ARGUMENT_NUMBER; i++)
				free(freenode->args[i]);
			free(freenode);
			return;
			
		}
		temp=temp->next;
	}
	
	fprintf(stderr,"%s: Node to be deleted does not exist (Error in line: %d)\n", __FILE__, __LINE__-21);

	free(temp);
	exit(-1);
}

struct list *list_search()
{
	struct list *temp = head;
	
	while(temp->next!=head)
	{
		if (temp->next->run==1) 
		{
			return(temp->next);
		}
		temp=temp->next;
	}
	
	fprintf(stderr,"%s: Could not find a running process (Error in line: %d)\n", __FILE__, __LINE__-13);
	exit(-1);
}

struct list *list_next()
{
	struct list *temp = head;
	
	while(temp->next!=head)
	{
		if (temp->next->run==1) 
		{
			if(temp->next->next==head)
				return(temp->next->next->next);
			return(temp->next->next);
		}
		temp=temp->next;
	}
	
	fprintf(stderr,"%s: Could not find a running process (Error in line: %d)\n", __FILE__, __LINE__-15);
	exit(-1);
}

int list_empty()
{
	if(head->next==head)
		return(0);
	else
		return(-1);
}

int list_print()
{
	int total_nodes=0, i;
	struct list *temp = head;
	
	while(temp->next!=head)
	{
		printf("pid: %d, name: (", temp->next->pid);
		
		for(i=0; i<ARGUMENT_NUMBER; i++)
		{	
			if(temp->next->args[i]==NULL)
				break;
			if(i==0)
				printf("%s ", temp->next->args[i]);
			else
				printf(", %s", temp->next->args[i]);
		}	
		
		if(temp->next->run==1)
			printf(") (R)\n");
		else
			printf(")\n");
		
		temp=temp->next;
		total_nodes++;
	}
	
	return(total_nodes);
}

int main(int argc, char *argv[])
{
	int i;
	char *args[ARGUMENT_NUMBER];
	
	args[0]=strdup("./test");
	for(i=1; i<5; i++)
		args[i]=strdup("a");
	
	args[5]=NULL;
		
	list_init();
	struct list *node = create_list_entry( 12345, args,  0);
	list_insert(node);
	struct list *node1 = create_list_entry( 54355, args,  1);
	list_insert(node1);
	struct list *node2 = create_list_entry( 545, args,  0);
	list_insert(node2);
	node=list_search();
	node=list_next();
	list_delete(545);
	
	printf("%d\n", list_print());
	
	free(head);
	
	return(0);
}
