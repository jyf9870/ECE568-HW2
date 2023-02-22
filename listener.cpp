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
#include "httpMethod.h"

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
    cout<<"request is:"<<endl;
    cout<<request<<endl;
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


    HttpMethod httpMethod;
    if(ctopRequest->getMethod() == "CONNECT"){
      httpMethod.connectRequest(server_fd,client_connection_fd);
    }
    if(ctopRequest->getMethod() == "GET"){
      //httpMethod.getRequest(server_fd,client_connection_fd);
      cout<<"getgetgetget!!!!!!"<<endl;
    }
    if(ctopRequest->getMethod() == "POST"){
      httpMethod.postRequest(server_fd,client_connection_fd);
      cout<<"postpostpostpost!!!!!!"<<endl;
    }

  }

  return 0;
}