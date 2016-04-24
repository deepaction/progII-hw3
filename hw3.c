#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define ARGUMENT_NUMBER 20
#define NAME_SIZE 512

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
		if(errno==ECHILD)
			return;
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

static void handler(int sig)
{
	volatile struct list *curr, *next;
	volatile int rv;
	
	if(head->next != head )
	{
		curr=list_search();
		next=list_next();
		
		rv=kill(curr->pid, SIGSTOP);
		failcheck(rv, __LINE__-1);
		
		curr->run = 0;
		
		rv=kill(next->pid, SIGCONT);
		failcheck(rv, __LINE__-1);
		
		next->run = 1;
		
		alarm(20);
		rv=write(1,"BEEP!\n",7);
		failcheck(rv, __LINE__-1);
	}
}

int main(int argc, char *argv[])
{
	int i=0, rv, flag=0;
	pid_t pid;
	char *args[ARGUMENT_NUMBER];
	char *command;
	char *arguments;
	char buf[NAME_SIZE];
	struct sigaction act={{0}}, alarm_act={{0}};
	struct list *temp, *curr;
// 	struct itimerval time={{0}};
	
	act.sa_handler=SIG_IGN;
	
	rv=sigaction(SIGUSR1, &act, NULL);
	failcheck(rv, __LINE__-1);
	
	alarm_act.sa_handler=handler;
	alarm_act.sa_flags = SA_RESTART;
	
	rv=sigaction(SIGALRM, &alarm_act, NULL);
	failcheck(rv, __LINE__-1);
	
// 	time.it_interval.tv_sec=20;
// 	time.it_interval.tv_usec=0;
// 	time.it_value.tv_sec=20;
// 	time.it_value.tv_usec=0;
// 		
// 	setitimer(ITIMER_REAL, &time, NULL);
	
	list_init();
	
	do
	{
		pid=waitpid(-1, NULL, WNOHANG);
		failcheck(pid, __LINE__-1);
		if(pid>0)
		{	
			if(head->next!=head)
			{
				temp=list_next();
			
				temp->run=1;
				rv=kill(temp->pid, SIGCONT);
				failcheck(rv, __LINE__-1);
				alarm(0);
			}
			list_delete(pid);
		}
		
		printf("$ ");
		fgets(buf, NAME_SIZE, stdin);
		command=strtok(buf, "  \n");
		
		pid=waitpid(-1, NULL, WNOHANG);
		failcheck(pid, __LINE__-1);
		if(pid>0)
		{	
// 			if(head->next!=head)
// 			{
// 				temp=list_next();
// 			
// 				temp->run=1;
// 				rv=kill(temp->pid, SIGCONT);
// 				failcheck(rv, __LINE__-1);
// 			}
			list_delete(pid);
		}
		
		if(command==NULL)
		{
			printf("Invalid command\n");
			continue;
		}
		
		if((strcmp(command, "quit")!=0)&&(strcmp(command, "info")!=0))
		{
			if((strcmp(command, "exec")!=0)&&(strcmp(command, "term")!=0)&&(strcmp(command, "sig")!=0))
			{
				printf("Invalid command\n");
				continue;
			}
			
			if(strcmp(command, "exec")==0)
			{
				i=0;
				arguments=command;
				while(arguments!=NULL)
				{
					arguments=strtok(NULL, " ");
					if(arguments==NULL)
						break;
					args[i]=strdup(arguments);
					i++;
				}
				args[i-1][strlen(args[i-1])-1]='\0';
				args[i]=NULL;
				
				pid=fork();
					
				if(pid==0)
				{
					act.sa_handler=SIG_DFL;
	
					rv=sigaction(SIGUSR1, &act, NULL);
					failcheck(rv, __LINE__-1);
					
					rv=execvp(args[0], args);
					failcheck(rv,__LINE__-1);
				}
				if(flag==0)
				{	
					list_insert(create_list_entry(pid, args,  1));
					alarm(20);
				}
				else
				{	
					rv=kill(pid, SIGSTOP);
					failcheck(rv, __LINE__-1);
					list_insert(create_list_entry(pid, args,  0));
				}
				flag=1;
			}
			else
			{
				pid=atoi(strtok(NULL, " "));
				if(strcmp(command, "term")==0)
				{
					rv=kill(pid, SIGTERM);
					failcheck(rv, __LINE__-1);
				}
				else if(strcmp(command, "sig")==0)
				{
					rv=kill(pid, SIGUSR1);
					failcheck(rv, __LINE__-1);
				}
			}
		}
		else if(strcmp(command, "info")==0)
		{	
			if(list_empty()!=0)
				list_print();
			else
				printf("There are currently no active processes\n");
		}
	}while((command==NULL)||(strcmp(command, "quit")!=0));
	
	temp=head->next;
	
	while(temp!=head)
	{
		curr=temp->next;
		
		rv=kill(temp->pid, SIGKILL);
		failcheck(pid, __LINE__-1);
		pid=waitpid(-1, NULL, WNOHANG);
		failcheck(pid, __LINE__-1);
		
		for(i=0; i<ARGUMENT_NUMBER; i++)
		{	
			free(temp->args[i]);
		}	
		
		free(temp);
		temp=curr;
	}
	head->next=head;
	
	//printf("%d\n", list_print());
	free(head);
	
	return(0);
}
