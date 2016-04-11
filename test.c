#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

volatile int i;

void failcheck(int rv, int line)
{
	if(rv<0)
	{
		fprintf(stderr,"%s: %s (Error in line: %d)\n", __FILE__, strerror(errno) , line);
		exit(-1);
	}
}

static void handler(int sig)
{
	printf("Reset to 0\n");
	i=0;
}


int main(int argc, char *argv[])
{
	int M, rv;
	pid_t pid;
	sigset_t set, p_set;
	struct sigaction act={{0}};
	
	if(argc!=5)
	{
		fprintf(stderr, "%s: Wrong number of arguments (Error in line: %d)\n", __FILE__, __LINE__);
		exit(-1);
	}
	if(strcmp(argv[1], "-m")!=0 || strcmp(argv[3], "-b")!=0)
	{
		fprintf(stderr, "%s: Expected -b and -m as arguments (Error in line: %d)\n", __FILE__, __LINE__);
		exit(-1);
	}
	
	act.sa_handler=handler;
	rv=sigaction(SIGUSR1, &act, NULL);
	failcheck(rv, __LINE__-1);
	
	M=atoi(argv[2]);
	pid=getpid();
	
	if(atoi(argv[4])==1)
	{	
		rv=sigemptyset(&set);
		failcheck(rv, __LINE__-1);
		rv=sigaddset(&set, SIGUSR1);
		failcheck(rv, __LINE__-1);
		rv=sigprocmask(SIG_BLOCK, &set, NULL);
		failcheck(rv, __LINE__-1);
	}
	
	for(i=0; i<=M; i++)
	{
		if(i==(int)(M/2) && atoi(argv[4])==1)
		{
			rv=sigprocmask(SIG_UNBLOCK, &set, NULL);
			failcheck(rv, __LINE__-1);
		}
		printf("%d: %d/%d\n", (int)pid, i, M);

// 		rv=write(1, &pid, sizeof(pid_t));
// 		failcheck(rv, __LINE__-1);
// 		rv=write(1, ": ", sizeof(char)*2);
// 		failcheck(rv, __LINE__-1);
// 		rv=write(1, &i, sizeof(int));
// 		failcheck(rv, __LINE__-1);
// 		rv=write(1, "/", sizeof(char));
// 		failcheck(rv, __LINE__-1);
// 		rv=write(1, &M, sizeof(int));
// 		failcheck(rv, __LINE__-1);
		sleep(5);
	}
	
	exit(0);
}
