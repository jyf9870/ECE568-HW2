#include <iostream>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "httpMethod.h"
#include "cache.h"
  

namespace http = boost::beast::http;

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
void HttpMethod::getRequest(int server_fd, int client_connection_fd, char buffer[], int length){
    Cache cache;
    send(server_fd, buffer,length,0); 
    char recvBuffer[100000];
    int length2 = recv(server_fd, recvBuffer, 100000, 0);
    // parseHttpResponse(recvBuffer, sizeof(recvBuffer));
    send(client_connection_fd,recvBuffer, length2, 0);
    while(length2 > 0){
        char new_recvBuffer[100000];
        length2 = recv(server_fd, new_recvBuffer, 100000, 0);
        if(length2 <= 0) break;
        // parseHttpResponse(recvBuffer, sizeof(recvBuffer));
        send(client_connection_fd,new_recvBuffer, length2, 0);
        cout<<length2<<endl; 
    }
    return;
}
   

void HttpMethod::parseHttpResponse(const char* res, size_t size) {
    boost::beast::flat_buffer buffer;
    http::response_parser<http::dynamic_body> parser;

    // Parse the HTTP response
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(res, size), ec);

    // Check for errors
    if (ec) {
        std::cout << "Error parsing HTTP response: " << ec.message() << std::endl;
        return;
    }

    // Check if the response has a chunk

    if (parser.get().chunked()) {
        std::cout << "Response has a chunk." << std::endl;
    } else {
        std::cout << "Response does not have a chunk." << std::endl;
    }

    // Get the last modified header
    auto iter = parser.get().find("Last-Modified");
    if (iter != parser.get().end()) {
        std::cout << "Last-Modified header: " << iter->value() << std::endl;
    } else {
        std::cout << "Last-Modified header not found." << std::endl;
    }
}


void HttpMethod::postRequest(int server_fd, int client_connection_fd){

    return;
}
