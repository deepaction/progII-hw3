#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define NAME_SIZE 256
#define ARGUMENT_NUMBER 20

struct list
{
	pid_t pid;
	char name[NAME_SIZE];
	char args[ARGUMENT_NUMBER][NAME_SIZE];
	int run;
	struct list *next;
} *head;

void list_init()
{
	head=(struct list *)malloc(sizeof(struct list));
	if(head==NULL)
	{
		fprintf(stderr,"%s: Error in dynamic memory allocation (Error in line: %d)\n", __FILE__, __LINE__-3);
		exit(-1);
	} 	
	head->next=head;
}

void list_insert(pid_t pid, char name[NAME_SIZE], char args[ARGUMENT_NUMBER][NAME_SIZE], int run)
{
	int i;
	struct list *curr;
	struct list *temp = head;
	
	curr=(struct list *)malloc(sizeof(struct list));
	if(curr==NULL)
	{
		fprintf(stderr,"%s: Error in dynamic memory allocation (Error in line: %d)\n", __FILE__, __LINE__-3);
		exit(-1);
	}
	curr->pid=pid;
	strcpy(curr->name, name);
	for(i=0; i<ARGUMENT_NUMBER; i++)
		strcpy(curr->args[i], args[i]);
	curr->run=run;
	
	if(head->next == head)
		head->next = curr;
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

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}




int main(int argc, char *argv[])
{
	
	
	return(0);
}