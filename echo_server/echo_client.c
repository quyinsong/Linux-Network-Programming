#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 6666
#define SERVER_IP "192.168.154.129"

int main(int argc, char *argv[]){

	int sockfd;
	char *message;
	struct sockaddr_in servaddr;
	int len;
	char buf[64];

	if(argc != 2){
		fputs("Usage: ./echo_client message \n", stderr);
		exit(1);
	}

	message = argv[1];

	printf("message: %s\n", message);

	sockfd = socket(AF_INET, SOCK_STREAM,0);

	memset(&servaddr, '\0',sizeof(struct sockaddr_in));

	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET,SERVER_IP,&servaddr.sin_addr);
	servaddr.sin_port = htons(SERVER_PORT);

	connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	write(sockfd,message,strlen(message));

	len = read(sockfd,buf,sizeof(buf));

	if (len>0){
		buf[len]='\0';
		printf("receive: %s\n",buf);
	
	}else{
		perror("error!!!");
	}

	printf("finished.");

	close(sockfd);
	
	

	return 0;
}
