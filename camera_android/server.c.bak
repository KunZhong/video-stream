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

/*
#define	DEBUG
*/
#define	MAX_BACKLOG	32
#define	MAX_BUF_LEN	4096

void http_process(int connfd);

ImageBuffer imgbuf;
pthread_t a_thread;
pthread_mutex_t mutex;


void *thread_function(void *arg)
{
	camera_capture(&imgbuf);
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
		http_process(connfd);
		close(connfd);
	}

	close(listenfd);
	exit(EXIT_SUCCESS);
}
void http_process(int connfd)
{
	int ret;
	char request[MAX_BUF_LEN];
	char response[MAX_BUF_LEN];

	memset(request, 0, sizeof(request));
	ret = read(connfd, request, MAX_BUF_LEN);
	if (ret <= 0) {
		if (ret == 0)
			printf("server->read: end-of-file\n");
		else
			perror("server->read");
		return;
	}
#ifdef DEBUG
	printf("server->read: the request is\n%s", request);
#endif
	if (strstr(request, "GET /index.html") != NULL) {
		int ret;
		FILE *fp;
		struct stat filestat;
		int filesize;
		char status[] = "HTTP/1.0 200 OK\r\n";
		char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
		char response_text[4096];

#ifdef DEBUG
		printf("server->http: get html\n");
#endif
		printf("server->http: get html\n");
		fp = fopen("index.html", "rb");
		if (fp == NULL) {
			perror("server->fopen");
			return;
		}

		ret = stat("index.html", &filestat);
		if (ret == -1) {
			perror("server->stat");
			return;
		}

		filesize = filestat.st_size;
		memset(response_text, 0, sizeof(response_text));
		if (fread(response_text, filesize, 1, fp) != 1) {
			fprintf(stderr, "server->fread: fread failure\n");
			fclose(fp);
			return;
		}

		memset(response, 0, sizeof(response));
		strcat(response, status);
		strcat(response, response_header);
		strcat(response, response_text);
#ifdef DEBUG
		printf("server->http: the response is\n%s\n", response);
#endif
		ret = write(connfd, response, strlen(response));
		if (ret == -1) {
			perror("server->write");
			fclose(fp);
			return;
		}

		fclose(fp);
	}

	if (strstr(request, "GET /?action=snapshot") != NULL) {
//		static int flag = 0 ;
		int count;
		char status[] = "HTTP/1.0 200 OK\r\n";
		char response_header[] = "Server: fsc100\r\nContent-Type: image/jpeg\r\nContent-Length: ";
		char length[32];

#ifdef DEBUG
		printf("server->http: get image\n");
#endif
		printf("server->http: get image\n");

		pthread_mutex_lock(&mutex);
	
		memset(response, 0, sizeof(response));
		strcat(response, status);
		strcat(response, response_header);
		snprintf(length, sizeof(length), "%d",imgbuf.length);
		strcat(response, length);
		strcat(response, "\r\n\r\n");
#ifdef DEBUG
		printf("server->http: the response is\n%s\n", response);
#endif
		ret = write(connfd, response, strlen(response));
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
		}	
		pthread_mutex_unlock(&mutex);
#ifdef DEBUG
		//printf("server->write: write %d bytes success\n", filesize);
#endif
	}

	if (strstr(request, "POST /index.html?video=") != NULL) {
		int ret;
		char status[] = "HTTP/1.0 200 OK\r\n";
		char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
		char response_text[32] = "video ";

		if (strstr(request, "video=on") != NULL) {
			/* you can turn on led here */

			//打开摄像头
			if(SUCCESS == camera_open()){	
			
//				pthread_video_create();
			
				printf("server->http: video on\n");
				strcat(response_text, "on");
			}
		}

		if (strstr(request, "video=off") != NULL) {
			/* you can turn on led here */

			//关闭摄像头
			if(SUCCESS == camera_Close()){

				//pthread_cancel(a_thread);
				printf("server->http: video off\n");
				strcat(response_text, "off");
			}
		}

		memset(response, 0, sizeof(response));
		strcat(response, status);
		strcat(response, response_header);
		strcat(response, response_text);
#ifdef DEBUG
		printf("server->http: the response is\n%s\n", response);
#endif
		ret = write(connfd, response, strlen(response));
		if (ret == -1) {
			perror("server->write");
			return;
		}
	}
}
