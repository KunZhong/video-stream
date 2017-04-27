/************************************************************************
	> File Name:	server-buf.c
	> Author:		zhongkun
	> Mail:			623933075@qq.com 
	> Created Time:	2016年08月03日 星期三 09时46分56秒
	> Motto:		ALL of the wine is inferior to you
 ************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>//struct sockaddr_in
#include <ctype.h>
#include <arpa/inet.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <core.h>

typedef struct sockaddr SA;
void setuparduino()
{
	pinMode(7,INPUT);
}
int main(int argc,char *argv[])
{
	int listenfd;
	int connfd;
	int n;//
	socklen_t peerlen;

	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	char buf[4096];

	if(argc != 2)
	{
		printf("Usge：%s portnumber\n",argv[0]);
		return 0;
	}

	setuparduino();

	if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	bzero(&servaddr,sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port= htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd,(SA *)&servaddr,sizeof(servaddr))<0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if(listen(listenfd,20)<0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	peerlen = sizeof(cliaddr);

	while(1)
	{
		//clilen = sizeof(cliaddr);//

		printf("server waiting...\n");
		
		if((connfd = accept(listenfd,(SA *)&cliaddr,&peerlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		printf("connection from [%s:%d]\n",(char *)inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

		while((n = read(connfd,buf,sizeof(buf)-1))>0)
		{
			buf[n] = 0;
			printf("echo :%s\n",buf);
			write(connfd,buf,n);
			//exit(-1);
		}
		printf("client is closed\n");
		close(connfd);
	}
	close(listenfd);
	return 0;
}
