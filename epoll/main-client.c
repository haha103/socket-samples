#include "socket-utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void * thr_func (void * args)
{
	char ** argv = (char **) args;
	pthread_t tid = pthread_self();

	int sfd, n;
	char send_buf[SOCK_BUF_SIZ];
	char recv_buf[SOCK_BUF_SIZ];

	if ((sfd = create_and_connect(argv[1], argv[2])) == -1) {
		perror("socket initialization failed");
		pthread_exit((void *)-1);
	}

	sprintf(send_buf, "Sending from client thread '%lu'", tid);
	
	if (write(sfd, send_buf, strlen(send_buf) + 1) == -1) {
		perror("write");
		pthread_exit((void *)-1);
	}
	if ((n = read(sfd, recv_buf, SOCK_BUF_SIZ)) == -1) {
		perror("read");
		pthread_exit((void *)-1);
	}

	return (void *)0;
}

int main (int argc, char ** argv)
{
	if (argc < 4) {
		fprintf(stderr, "Usage: %s [server ip address] [server port] [num of threads]\n", argv[0]);
		return EXIT_FAILURE;
	}

	int thread_count = atoi(argv[3]);

	pthread_t * tids = (pthread_t *) malloc(thread_count * sizeof (pthread_t));
	int * retvals = (int *) malloc (thread_count * sizeof (int));

	for (int i = 0; i < thread_count; ++i) {
		if (pthread_create(&tids[i], NULL, thr_func, (void *)argv)) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < thread_count; ++i) {
		if (pthread_join(tids[i], (void **)&retvals[i])) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < thread_count; ++i) {
		printf("Thread '%lu' returned: %d\n", tids[i], retvals[i]);
	}

	free(tids);
	free(retvals);
	
	return EXIT_SUCCESS;
}
