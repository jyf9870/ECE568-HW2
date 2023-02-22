#include <iostream>
#include "httpMethod.h"
  
void HttpMethod::connectRequest(int server_fd, int client_connection_fd){
    send(client_connection_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    fd_set read_fd;
    int selector = server_fd > client_connection_fd ? server_fd + 1 : client_connection_fd + 1;

    while (1) {
        FD_ZERO(&read_fd);
        FD_SET(server_fd, &read_fd);
        FD_SET(client_connection_fd, &read_fd);

        select(selector, &read_fd, NULL, NULL, NULL);
        int fd[2] = {server_fd, client_connection_fd};
        int length;
        for (int i = 0; i < 2; i++) {
            char message[100000];
            if (FD_ISSET(fd[i], &read_fd)) {
                length = recv(fd[i], message, sizeof(message), 0);
                cout<<message<<endl;
                if (length <= 0) {
                    return;
                }
                else {
                    if (send(fd[1 - i], message, length, 0) <= 0) {
                        return;
                    }
                }
            }
        }
    }
}
void HttpMethod::getRequest(int server_fd, int client_connection_fd){
    //sent request to server
    //send(server_fd, buffer,length,0); 
    char recvBuffer[100000];
    int length = recv(server_fd, recvBuffer, 100000, 0);
    send(client_connection_fd,recvBuffer, length, 0);
    // cout<<"response is:"<<endl;
    // cout<<recvBuffer<<endl;

    return;
}
void HttpMethod::postRequest(int server_fd, int client_connection_fd){

    return;
}
