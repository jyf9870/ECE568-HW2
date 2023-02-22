#include <iostream>
#include "httpMethod.h"
  
void HttpMethod::connectRequest(int server_fd, int client_connection_fd){
    send(client_connection_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

    while (true) {
        fd_set read_fd;
        int selector = server_fd > client_connection_fd ? server_fd: client_connection_fd;

        FD_ZERO(&read_fd);
        FD_SET(server_fd, &read_fd);
        FD_SET(client_connection_fd, &read_fd);
        int res = select(selector  + 1, &read_fd, NULL, NULL, NULL);
        if (res <= 0) return;
        
        int fd[2] = {server_fd, client_connection_fd};
        int length;
        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(fd[i], &read_fd)) {
                int ll = i == 0 ? 100000 : 65536;
                char message[ll];
                length = recv(fd[i], message, sizeof(message), 0);
                if (length > 0) {
                    if (send(fd[1 - i], message, length, 0) <= 0) return;     
                }
            else{
                return;
                }
            }
        }
    }
}
void HttpMethod::getRequest(int server_fd, int client_connection_fd){

    // cout<<"response is:"<<endl;
    // cout<<recvBuffer<<endl;

    return;
}
void HttpMethod::postRequest(int server_fd, int client_connection_fd){

    return;
}
