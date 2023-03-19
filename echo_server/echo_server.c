#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERVER_PORT 6666

void do_http_request(int client_sock);
int get_line(int client_socket, char *buf, int size);

int main(void){
	
	int sock;// 代表信箱
	struct sockaddr_in server_addr;
	// 创建一个信箱
	sock = socket(AF_INET,SOCK_STREAM, 0);
	
	printf("sock: %d\n",sock);
	// 清空标签，写上地址和端口号
	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;// 选择协议IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听本体所有IP地址
	server_addr.sin_port = htons(SERVER_PORT); // 绑定端口号
	
	// 实现标签贴到收信的信箱上
	bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
	
	// 把信箱挂至到传达室
	int l = listen(sock,128);

	printf("listen: %d\n",l);
	// 万事俱备，只等来信
	printf("等待客户端的连接\n");

	int done = 1;

	while(done){
		struct sockaddr_in client;
		int client_sock, len;
		char client_ip[64];
		char buf[256];

		socklen_t client_addr_len;
		client_addr_len = sizeof(client);
		
		//阻塞，直至有新的客户端连接
		client_sock = accept(sock,(struct sockaddr *)&client,&client_addr_len);

		printf("client_sock: %d\n",client_sock);	
		// 打印客户端IP地址和端口号
		printf("client ip: %s\t port: %d\n",
			inet_ntop(AF_INET,&client.sin_addr.s_addr,client_ip,
			sizeof(client_ip)),
			ntohs(client.sin_port));

		//读取客户端发送的数据
		len = read(client_sock,buf,sizeof(buf)-1);
		buf[len] = '\0';
		printf("receive: %s\n", buf);
		//向客户端发送数据
		len = write(client_sock, buf, len);
		printf("write: %s: len: %d\n",buf,len);
		//关闭socket
		//close(client_sock);
	}

	close (sock);
	
	return 0;
}














