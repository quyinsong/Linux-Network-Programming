#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERVER_PORT 80

void do_http_request(int client_sock);
int get_line(int client_socket, char *buf, int size);

int main(void){
	
	int sock;// 代表信箱
	struct sockaddr_in server_addr;
	// 美女创建一个信箱
	sock = socket(AF_INET,SOCK_STREAM, 0);
	
	// 清空标签，写上地址和端口号
	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;// 选择协议IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听本体所有IP地址
	server_addr.sin_port = htons(SERVER_PORT); // 绑定端口号
	
	// 实现标签贴到收信的信箱上
	bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
	
	// 把信箱挂至到传达室
	listen(sock,128);

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
		client_sock = accept(sock,(struct sockaddr *)&client,&client_addr_len);
	
		// 打印客户端IP地址和端口号
		printf("client ip: %s\t port: %d\n",
			inet_ntop(AF_INET,&client.sin_addr.s_addr,client_ip,
			sizeof(client_ip)),
			ntohs(client.sin_port));

		//处理http请求，读取客户端发送的数据
		do_http_request(client_sock);
		
		close(client_sock);
		
	}

	close (sock);
	
	return 0;
}

void do_http_request(int client_sock){
	int len = 0;
	char buf[256];

	/*读取客户端发送的http请求*/
	//1.  读取请求行
	do{
		len = get_line(client_sock,buf,sizeof(buf));
		printf("read line: %s\n",buf);
	}while(len > 0);
	len = get_line(client_sock,buf,sizeof(buf));
	
}

// 返回值： -1 表示读取出错，等于0表示读到一个空行，大于0表示成功读取一行
int get_line(int client_sock, char *buf, int size){
	
	int count = 0;
	char ch = '\0';
	int len = 0;

	while((count < size-1) && ch != '\n'){
		len = read(client_sock, &ch, 1);

		if(len == 1){
			if(ch == '\r')	continue;
			buf[count] = ch;
			count++;
		}else if( len == -1){
			perror("read failed");
			count = -1;
			break;
		}else{
			fprintf(stderr, "client close.\n");
			count = -1;
			break;
		}
		if (count >= 0) buf[count] = '\0';
	}

} 














