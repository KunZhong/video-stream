#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "camera.h"


#define	DEBUG

#define	MAX_BACKLOG	32
#define	MAX_BUF_LEN	4096

void app_process(int connfd);

ImageBuffer imgbuf;
pthread_t a_thread;
pthread_mutex_t mutex;


void *thread_function(void *arg)
{
	camera_capture(&imgbuf);
}
	
void *serve_android(void *arg)
{
	int connfd = (int)arg;
	while(1){
	app_process(connfd);
	}
}

int main(int argc, char *argv[])
{
	int ret;
	int listenfd;
	int opt = 1;
	int connfd;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t addrlen;
	unsigned short port;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		perror("server->socket");
		exit(EXIT_FAILURE);
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	port = atoi(argv[1]);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("server->bind");
		close(listenfd);
		exit(EXIT_FAILURE);
	}

	ret = listen(listenfd, MAX_BACKLOG);
	if (ret == -1) {
		perror("server->listen");
		close(listenfd);
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&mutex,NULL)<0)
	{
		perror("fail to mutex_init");
		exit(-1);
	}

	int pid;
	pid = pthread_create(&a_thread,NULL,thread_function,(void*)NULL);
	if(pid < 0)
	{
		perror("fail to pthread_create!\n");
		exit(-1);
	}

	while (1) {
		addrlen = sizeof(struct sockaddr_in);
		memset(&cliaddr, 0, sizeof(struct sockaddr_in));
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &addrlen);
		if (connfd == -1) {
			perror("server->accept");
			continue;
		}
#ifdef DEBUG
		printf("server->accept: a new client is comming, and the connfd is %d\n", connfd);
#endif
		pthread_t thread;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&thread,&attr,serve_android,(void *)connfd);	

	}

	close(listenfd);
	exit(EXIT_SUCCESS);
}
void app_process(int connfd)
{
	int ret;
	char request[MAX_BUF_LEN];
	char response[MAX_BUF_LEN];

	memset(request, 0, sizeof(request));
	ret = read(connfd, request, MAX_BUF_LEN);
	if (ret < 0) {
		//
		//if (ret == 0)
			//printf("server->read: end-of-file\n");
		//else
			perror("server->read");
		//return;
	}
#ifdef DEBUG
	printf("server->read: the request is\n%s", request);
#endif

	if(strncmp(request,"GET_IMG",7) == 0)
	{
		char length[20];
		int count;
		
		pthread_mutex_lock(&mutex);
	
		memset(length, 0, sizeof(length));
		snprintf(length, sizeof(length), "%d",imgbuf.length);
		strcat(length, "len");
	
		printf("img:%s\n",length);

		ret = write(connfd, length, sizeof(length));
		if (ret == -1) {
			perror("server->write");
			return;
		}	
		count = 0;
		while (count < imgbuf.length) {
			ret = write(connfd, imgbuf.start + count, imgbuf.length - count);
			if (ret == -1) {
				perror("server->write");
				return;
			}
			count += ret;
			printf("count = %d\n",count);
		}	
		pthread_mutex_unlock(&mutex);
	}
}
