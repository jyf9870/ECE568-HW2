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


using namespace std;

int main(int argc, char *argv[])
{
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = "8000";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);


  /*
  * this is an error handing part
  */
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } 

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  
  /*
  * this is an error handing part
  */
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } 

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  /*
  * this is an error handing part
  */
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } 

  status = listen(socket_fd, 100);
  /*
  * this is an error handing part
  */
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } 

  while(true){
    cout << "Waiting for connection on port " << port << endl;
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    /*
    * this is an error handing part
    */
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    } 

    char buffer[65536];
    int length = recv(client_connection_fd, buffer, 1024, 0);
    string request = string(buffer, length);
    Request * ctopRequest = new Request(request);

    /*
    * this is an error handing part
    */
    if (ctopRequest->getMethod() != "POST" && ctopRequest->getMethod() != "GET" && ctopRequest->getMethod() != "CONNECT") {
      const char * req400 = "HTTP/1.1 400 Bad Request";
      //TODO!!!: write something into the log file!
      return -1;
    }

 
    string hostname = ctopRequest->getRequestMap().find("Host")->second;
    cout<<hostname.c_str()<<endl;

    Socket socket;
    int server_fd = socket.connectToServer(hostname.c_str());
 
  }
  //when finish the proxy, close the socket 
  // freeaddrinfo(host_info_list);
  // close(socket_fd);

  return 0;
}