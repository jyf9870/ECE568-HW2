#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include "request.h"
#include "socket.h"
#include "requirement.h"


using namespace std;

int main(int argc, char *argv[])
{
  Socket socket;
  int socket_fd = socket.connectToClient(); //connect to client
  while(true){
    cout << "Waiting for connection on port " << socket.port << endl;
    int client_connection_fd = socket.acceptToClient(socket_fd); //accepted

    char buffer[65536];
    int length = recv(client_connection_fd, buffer, 65536, 0);
    string request = string(buffer, length);
    Request * ctopRequest = new Request(request); //handle requests

    /*
    * this is an error handing part
    */
    if (ctopRequest->getMethod() != "POST" && ctopRequest->getMethod() != "GET" && ctopRequest->getMethod() != "CONNECT") {
      const char * req400 = "HTTP/1.1 400 Bad Request";
      //TODO!!!: write something into the log file!
      return -1;
    }

    string hostname = ctopRequest->getRequestMap().find("Host")->second;
    int server_fd = socket.connectToServer(hostname.c_str(),ctopRequest->getPort().c_str()); //connected successfully from client to proxy and proxy to server
    
    //sent/receive request to/from server, send it to client
    send(server_fd, buffer,length,0); 
    char recvBuffer[100000];
    length = recv(server_fd, recvBuffer, 100000, 0);
    send(client_connection_fd,recvBuffer, length, 0);

    // cout<<"request from clinet:"<<endl;
    // cout<<buffer<<endl;
    // cout<<"proxy received from server:"<<endl;
    // cout<<length<<endl;
    // cout<<"request is:"<<endl;
    // cout<<request<<endl;
    cout<<"response is:"<<endl;
    cout<<recvBuffer<<endl;

    if(ctopRequest->getMethod() == "CONNECT"){
      socket.connectRequest(server_fd,client_connection_fd);
    }


 
  }
  //when finish the proxy, close the socket 
  // freeaddrinfo(host_info_list);
  // close(socket_fd);

  return 0;
}