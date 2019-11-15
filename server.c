#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define backlog 5
#define MAXLINE 10
#define SA struct sockaddr 
#define MAX_TOPIC_SIZE 32
#define MAX_MESSAGE_SIZE 512


void str_server(int connfd);
void perror2(char *s);

typedef struct my_struct{
    int type;
    char message[MAX_MESSAGE_SIZE];
    char topic_name[MAX_TOPIC_SIZE];
}to_broker;

void perror2(char *s){
	perror(s);
	exit(0);
}


void str_server(int connfd){

	int x1,x2;
	//char buf[MAXLINE];
    to_broker t1;
	while((x1=read(connfd, &t1, sizeof(t1)))>0){
		printf("type : %d : message : %s : topic name: %s\n",t1.type,t1.message,t1.topic_name );	
		//if((x2=write(connfd, buf, x1))==-1)
		//	perror2("write in str_server");
	}

	if(x1<0)
		perror2("read in str_server");
}




int main(int argc, char *argv[]){

	int listenfd,connfd;
	struct sockaddr_in servaddr,cliaddr;
	int pid;
	socklen_t clilen;
	char buf[INET_ADDRSTRLEN];

	if((listenfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("socket server: ");
		exit(0);
	}

	bzero(&cliaddr,sizeof(cliaddr));
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family= AF_INET;
	servaddr.sin_port= htons(1900);
	servaddr.sin_addr.s_addr= htonl(INADDR_ANY);

	if(bind(listenfd, (SA *) &servaddr, sizeof(servaddr))==-1){
		perror("server bind: ");
		exit(0);
	}

	listen(listenfd, backlog);

	for(;;){

		printf("Calling accept()\n");
		clilen=sizeof(cliaddr);
		if( (connfd=accept(listenfd, (SA *) &cliaddr, &clilen) )==-1)
			perror("connect: ");
		else{ 
			if(inet_ntop(AF_INET, &cliaddr.sin_addr, buf,INET_ADDRSTRLEN)==NULL){
				perror("inet_ntop : ");
			}
			printf("Client Connected. Its IP: %s  Port: %d\n",buf,ntohs(cliaddr.sin_port));
		}

		if((pid=fork())==-1)
			perror("fork: ");

		if(pid==0){
			close(listenfd);
			str_server(connfd);
			exit(0);
		}

		close(connfd);
	}

	return 0;
}