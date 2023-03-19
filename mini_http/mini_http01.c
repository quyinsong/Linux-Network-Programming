#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>

#define SERVER_PORT 80
char *SERVER_IP = "192.168.154.129";
static int debug = 1;

void* do_http_request(void *pclient_sock);
void do_http_response(int client_sock);
void do_http_response01(int client_sock, const char *path);
int header(int client_sock, FILE *resource);
void cat(int client_sock, FILE *resource);
void inner_error(int client_sock);
void not_found(int client_sock);
void unimplemented(int client_sock);
void bad_request(int client_sock);
int get_line(int client_socket, char *buf, int size);

int main(void){
	
	int sock;// 代表信箱
	struct sockaddr_in server_addr;
	// 美女创建一个信箱
	sock = socket(AF_INET,SOCK_STREAM, 0);
	
	// 清空标签，写上地址和端口号
	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;// 选择协议IPV4
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听本体所有IP地址
	inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr.s_addr);
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
		pthread_t id;
		int* pclient_sock = NULL;

		socklen_t client_addr_len;
		client_addr_len = sizeof(client);
		client_sock = accept(sock,(struct sockaddr *)&client,&client_addr_len);
	
		// 打印客户端IP地址和端口号
		printf("client ip: %s\t port: %d\n",
			inet_ntop(AF_INET,&client.sin_addr.s_addr,client_ip,
			sizeof(client_ip)),
			ntohs(client.sin_port));

		//处理http请求，读取客户端发送的数据
		//do_http_request(client_sock);
		
		//启动线程处理http请求
		pclient_sock = (int *)malloc(sizeof(int));
		*pclient_sock = client_sock;
		
		pthread_create(&id, NULL, do_http_request, (void *)pclient_sock);
	
	}

	close(sock);
	
	return 0;
}
//获得http请求
void* do_http_request(void* pclient_sock){
	int len = 0;
	char buf[4096];
	char method[64];
	char url[256];
	char path[256];
	int client_sock = *(int *)pclient_sock;

	struct stat st;

	int i = 0, j = 0;

	/*读取客户端发送的http请求*/
	//1.  读取请求行
	len = get_line(client_sock,buf,sizeof(buf));
	if(len > 0){
		while(!isspace(buf[j]) && (i< sizeof(method)-1)){
			method[i] = buf[j];
			i++;
			j++;
		}
		method[i] = '\0';
		if(debug) printf("request method: %s\n",method);

		if(strncasecmp(method,"GET",strlen(method))==0){//只处理get请求
			if(debug) printf("request method = GET\n");
	
			//获取url
			while(isspace(buf[j++])); //跳过空格
			i = 0;

			while(!isspace(buf[j]) && (i<sizeof(url)-1)){
				url[i] = buf[j];
				i++;
				j++;
			}
			url[i] = '\0';
			if(debug) printf("url: %s\n", url);
			// 继续读取http头部
			//do{
			//	len = get_line(client_sock,buf,sizeof(buf));
			//	if(debug) printf("read: %s\n", buf);
			//}while(len>0);
			//len = get_line(client_sock,buf,sizeof(buf));
			if(debug) printf("read: %s\n", buf);
		
			//***定位服务器本地文件***
			
			//处理url 中的 ？
			{
				char *pos = strchr(url,'?');
				if(pos) {
					*pos = '\0';
					printf("real url: %s\n", url);	
			
				}
			}
			sprintf(path, "./html_docs/%s", url);
			if(debug) printf("path: %s\n", path);
			
			//执行http响应
			//判断文件是否存在，如果存在就响应200 OK， 同时发送相应的html文件，如果不存在就响应404 NOT FOUND
			if(stat(path,&st)==-1){// 文件不存在，报错404
				fprintf(stderr, "stat %s failed. reason: %s\n",path, strerror(errno));
				not_found(client_sock); //404
			}else {//文件存在

				if(S_ISDIR(st.st_mode)){
					strcat(path, "/index.html");
				}
				//do_http_response(client_sock);
				do_http_response01(client_sock,path);
			}
			
		} else { //非get请求，读取所有http头部，并响应客户端501 method not implement
			fprintf(stderr, "warning! other request [%s]\n", method);
			do{
				len = get_line(client_sock,buf,sizeof(buf));
				if(debug) printf("read: %s\n", buf);
			}while(len > 0);
			
			unimplemented(client_sock); //在响应时再实现
		        } 
	}else {// 请求格式有问题，出错处理
		bad_request(client_sock); //在响应时再处理
	      }
		  
	close(client_sock);
	if(pclient_sock) free(pclient_sock); //释放动态分配的内存 
	return NULL;
}

//执行http响应
void do_http_response(int client_sock){
	const char * main_header = "HTTP/1.0 200 OK\r\nServer: QYS Server\r\nContent-Type: text/html\r\nConnection: Close\r\n";
	
	const char * welcome_content = "<!DOCTYPE html>\n\
	<html lang = \"ch-CN\">\n\
	<head>\n\
	<meta content=\"text/html; charset = utf-8\" http-equiv = \"Content-Type\">\n\
	<title> This is a test</title>\n\
	</head>\n\
	<body>\n\
	<div align=center height=\"500px\">\n\
	<br/><br/><br/>\n\
	<h2>大家好，欢迎来到大白的mini-http网页！</h2><br/><br/>\n\
	<form action=\"commit\" method = \"post\">\n\
	尊姓大名：<input type=\"text\" name = \"name\" />\n\
	<br/>芳龄几何: <input type = \"password\" name=  \"age\" />\n\
	<br/><br/><br/><input type = \"submit\" value = \"提交\" />\n\
	<input type = \"reset\" value = \"重置\" />\n\
	</form>\n\
	</div>\n\
	</body>\n\
	</html>";


	//1. 送main_header
	int len = write(client_sock, main_header,strlen(main_header));

	if(debug) fprintf(stdout, "... do http_response...\n");
	if(debug) fprintf(stdout, "write[%d]: %s\n", len, main_header);

	//2. 生成 Content_Length 行并发送
	char send_buf[128];
	int wc_len = strlen(welcome_content);
	len = snprintf(send_buf,128,"Content-Length: %d\r\n\r\n", wc_len);
	len = write(client_sock,send_buf,len);

	if(debug) fprintf(stdout, "write [%d]: %s\n", len, send_buf);

	len = write(client_sock, welcome_content, wc_len);
	
	if(debug) fprintf(stdout, "write [%d]: %s\n", len, welcome_content);
}

void do_http_response01(int client_sock, const char *path){
	int ret = 0;
	FILE *resource = NULL;

	resource = fopen(path, "r");

	if(resource == NULL){
		not_found(client_sock);
		return ;
	}
	if(debug) fprintf(stdout, "... do http_response...\n");
	// 1. 发送http头部
	ret = header(client_sock, resource);
	// 2. 发送http body
	if(!ret){
		cat(client_sock, resource);
	}

	fclose(resource);
}

//发送http头部
int header(int client_sock, FILE *resource){
	struct stat st;
	int fileid = 0;
	char tmp[128];
	char buf[2048]={0};
	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	strcat(buf, "Server: QYS Server\r\n");
	strcat(buf, "Connection-Type: text/html\r\n");
	strcat(buf, "Connection: Close\r\n");

	fileid = fileno(resource);

	if(fstat(fileid, &st)== -1){
		inner_error(client_sock);
		return -1;
	}
	
	snprintf(tmp,128,"Content-Length: %ld\r\n\r\n",st.st_size);
	strcat(buf,tmp);

	if(debug) fprintf(stdout, "header: %s\n",buf);

	if(write(client_sock,buf,strlen(buf))<0){
		fprintf(stderr, "send failed. data: %s,reason:%s\n", buf, strerror(errno));
		return -1;
	}
	return 0;

	
}

//发送http本体
void cat(int client_sock, FILE *resource){
	char buf[2048];
	int i = 0;

	while(!feof(resource)){
		fgets(buf,sizeof(buf),resource); //注意strlen与sizeof的区别
		int len = write(client_sock,buf,strlen(buf)); //修改：sizeof->strlen,修改之前无法访问

		if(len<0){
			fprintf(stderr,"send body error. reason: %s\n",strerror(errno));
			break;
		}
		i++;
		if(debug) fprintf(stdout, "body[%d]: %s",i,buf);
		
	}
}


//服务器内部错误
void inner_error(int client_sock){
	const char * reply = "HTTP/1.0 500 Internal Sever Error\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\r\n\
<HEAD>\r\n\
<TITLE>Method Not Implemented</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>Error prohibited CGI execution.\r\n\
</BODY>\r\n\
</HTML>";
	int len = write(client_sock,reply,strlen(reply));
	if(debug) fprintf(stdout, reply);

	if(len<=0){
		fprintf(stderr, "send reply failed.reason:%s\n", strerror(errno));
	}
}

//404 NOT FOUND
void not_found(int client_sock){
	const char * reply = "HTTP/1.0 404 NOT FOUND\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML lang = \"zh-CN\">\r\n\
<meta content = \"text/html; charset = utf-8\" http-equiv=\"Content-Type\">\r\n\
<HEAD>\r\n\
<TITLE>NOT FOUND</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <p>文件不存在!!!\r\n\
    <P>The server could not fulfill your request because the resource specified is unavailable or nonexistent.\r\n\
</BODY>\r\n\
</HTML>";

	int len = write(client_sock,reply,strlen(reply));
	if(debug) fprintf(stdout, reply);

	if(len<=0){
		fprintf(stderr, "send reply failed. reson: %s\n", strerror(errno));
	}


}

//501 method not implemented
void unimplemented(int client_sock){
	
	const char *reply = "HTTP/1.0 501 Method Not Implemented\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\r\n\
<HEAD>\r\n\
<TITLE>Method Not Implemented</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>HTTP request method not supported.\r\n\
</BODY>\r\n\
</HTML>";
	int len = write(client_sock,reply,strlen(reply));
	if(debug) fprintf(stdout, reply);

	if(len<=0){
		fprintf(stderr, "method implemented failed. reson: %s\n", strerror(errno));
	}

}

//400 bad request
void bad_request(int client_sock){
	
	const char *reply = "HTTP/1.0 400 BAD REQUEST\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\r\n\
<HEAD>\r\n\
<TITLE>BAD REQUEST</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>Your browser sent a bad request！\r\n\
</BODY>\r\n\
</HTML>";
	int len = write(client_sock,reply,strlen(reply));
	if(debug) fprintf(stdout, reply);

	if(len<=0){
		fprintf(stderr, "bad request. reson: %s\n", strerror(errno));
	}

}

/*
void do_http_response(int client_sock){
    const char *main_header = "HTTP/1.0 200 OK\r\nServer: Martin Server\r\nContent-Type: text/html\r\nConnection: Close\r\n";


    const char * welcome_content = "\
<html lang=\"zh-CN\">\n\
<head>\n\
<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n\
<title>This is a test</title>\n\
</head>\n\
<body>\n\
<div align=center height=\"500px\" >\n\
<br/><br/><br/>\n\
<h2>大家好，欢迎来到奇牛学院VIP 试听课！</h2><br/><br/>\n\
<form action=\"commit\" method=\"post\">\n\
尊姓大名: <input type=\"text\" name=\"name\" />\n\
<br/>芳龄几何: <input type=\"password\" name=\"age\" />\n\
<br/><br/><br/><input type=\"submit\" value=\"提交\" />\n\
<input type=\"reset\" value=\"重置\" />\n\
</form>\n\
</div>\n\
</body>\n\
</html>";

    char send_buf[64];
    int wc_len = strlen(welcome_content);
    int len = write(client_sock, main_header, strlen(main_header));

    if(debug) fprintf(stdout, "... do_http_response...\n");
    if(debug) fprintf(stdout, "write[%d]: %s", len, main_header);

    len =snprintf(send_buf, 64,"Content-Length: %d\r\n\r\n", wc_len);
    len = write(client_sock, send_buf, len);
    if(debug) fprintf(stdout, "write[%d]: %s", len, send_buf);

    len = write(client_sock, welcome_content, wc_len);
    if(debug) fprintf(stdout, "write[%d]: %s", len, welcome_content);

}
*/



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
			fprintf(stderr, "client close...\n");
			count = -1;
			break;
		}
	}

	if (count >= 0) buf[count] = '\0';
	return count;
} 


/*
//返回值： -1 表示读取出错， 等于0表示读到一个空行， 大于0 表示成功读取一行
int get_line(int sock, char *buf, int size){
	int count = 0;
	char ch = '\0';
	int len = 0;


	while( (count<size - 1) && ch!='\n'){
		len = read(sock, &ch, 1);

		if(len == 1){
			if(ch == '\r'){
				continue;
			}else if(ch == '\n'){
				//buf[count] = '\0';
				break;
			}

			//这里处理一般的字符
			buf[count] = ch;
			count++;

		}else if( len == -1 ){//读取出错
			perror("read failed");
			count = -1;
			break;
		}else {// read 返回0,客户端关闭sock 连接.
			fprintf(stderr, "client close.\n");
			count = -1;
			break;
		}
	}

	if(count >= 0) buf[count] = '\0';

	return count;
}
*/












