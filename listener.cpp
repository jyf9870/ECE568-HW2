#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>

int main(){
    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  	// AF_INET :   表示使用 IPv4 地址		可选参数
    // SOCK_STREAM 表示使用面向连接的数据传输方式，
    // IPPROTO_TCP 表示使用 TCP 协议

    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("174.109.104.90");  //具体的IP地址
    serv_addr.sin_port = htons(8000);  //端口
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);

    while(true){
      printf("wait for client!\n");
      //接收客户端请求
      struct sockaddr_in clnt_addr;
      socklen_t clnt_addr_size = sizeof(clnt_addr);
      int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

      //read the client data
      char se_get[1024];
      read(clnt_sock,se_get,sizeof(se_get));
      // for(int i = 0; i < 1024; i++){
        std::string s = "";
        int i = 0;
      while (se_get[i] != '\n') {
        s += se_get;
        std::cout << s << std::endl;
        i++;
      }
       std::cout << se_get << std::endl;
      //向客户端发送数据
      // char str[] = "Hello World!";
      // write(clnt_sock, str, sizeof(str));
      
    
   
    //关闭套接字
    // close(clnt_sock);
    // close(serv_sock);

    return 0;
}
}
