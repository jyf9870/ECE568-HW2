#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "httpMethod.hpp"
#include "request.h"
#include "socket.h"

static Cache cache_map;
pthread_mutex_t listenerMutex = PTHREAD_MUTEX_INITIALIZER;

using namespace std;

class Client_Info {
 public:
  int client_connection_fd;
  int client_id;
  Socket socket;

  Client_Info(int _client_connection_fd,
              int _client_id,
              Socket _socket) :
      client_connection_fd(_client_connection_fd),
      client_id(_client_id),
      socket(_socket) {}
};

void * request_handle(void * client_info) {
  Client_Info * curr_client_info = (Client_Info *)client_info;
  cout << "into a new thread" << endl;
  int client_connection_fd = curr_client_info->client_connection_fd;

  char buffer[65536];
  cout << "waiting for the request from client!" << endl;
  int length = recv(client_connection_fd, buffer, 65536, 0);
  string request = string(buffer, length);
  Request * ctopRequest = new Request(request);  //handle requests

  /*
    * this is an error handing part
    */
  if (ctopRequest->getMethod() != "POST" && ctopRequest->getMethod() != "GET" &&
      ctopRequest->getMethod() != "CONNECT") {
    string req400 = "HTTP/1.1 400 Bad Request";
    stringstream bad_request_ss;
    bad_request_ss << curr_client_info->client_id << ": Responding \"" << req400 << "\"";
    addToLog(
        bad_request_ss
            .str());  // 	If the	proxy	receives	a	malformed	request,	it	MUST	reply	with	a	400	error	code.
    //send(client_connection_fd, )
    return NULL;
  }

  stringstream request_ss;
  request_ss << curr_client_info->client_id << ": \"" << ctopRequest->getRequesLine()
             << "\" from " << getClientIP(client_connection_fd) << " @ "
             << ctopRequest->getTime();
   addToLog(request_ss.str());  // add to log: ID: "REQUEST" from IPFROM @ TIME;

  string hostname = ctopRequest->getRequestMap().find("Host")->second;
  int server_fd = curr_client_info->socket.connectToServer(
      hostname.c_str(),
      ctopRequest->getPort()
          .c_str());  //connected successfully from client to proxy and proxy to server

  HttpMethod httpMethod;
  if (ctopRequest->getMethod() == "CONNECT") {
    cout << "I am handling connect TAT!!!!!!" << endl;
    httpMethod.connectRequest(server_fd,
                              client_connection_fd,
                              curr_client_info->client_id,
                              ctopRequest->getRequesLine(),
                              ctopRequest->get_server_hostname());
    cout << "CONNECTCONNECTCONNECTCONNECT!!!!!!" << endl;

    stringstream tunnel_close_ss;
    tunnel_close_ss << curr_client_info->client_id << ": Tunnel closed";
    addToLog(tunnel_close_ss.str());              // ID: Tunnel closed

    return NULL;
  }

  if (ctopRequest->getMethod() == "GET") {
    cout << "I am handling get TAT!!!!!!" << endl;
    httpMethod.getRequest(server_fd,
                          client_connection_fd,
                          buffer,
                          length,
                          cache_map,
                          curr_client_info->client_id,
                          ctopRequest->getRequesLine(),
                          ctopRequest->get_server_hostname());
    cout << "getgetgetget!!!!!!" << endl;
    return NULL;
  }

  if (ctopRequest->getMethod() == "POST") {
    cout << "I am handling post TAT!!!!!!" << endl;
    httpMethod.postRequest(server_fd,
                           client_connection_fd,
                           buffer,
                           length,
                           curr_client_info->client_id,
                           ctopRequest->getRequesLine(),
                           ctopRequest->get_server_hostname());
    cout << "postpostpostpost!!!!!!" << endl;
    return NULL;
  }

  close(server_fd);
  close(client_connection_fd);

  return NULL;
}

int main(int argc, char * argv[]) {
  Socket socket;
  int socket_fd = socket.connectToClient();  //connect to client
  if (socket_fd == -1) {
    const string message = "Proxxy can not create server socket for the client!";
    cout << message << endl;
    addToLog(message);
    return -1;
  }
  int client_id = 0;
  while (true) {
    cout << "Waiting for connection on port " << socket.port << endl;
    int client_connection_fd = socket.acceptToClient(socket_fd);  //accepted

    if (client_connection_fd == -1) {  //"(no-id): ERROR in connecting client"
      addToLog("(no-id): ERROR in connecting client");
      continue;
    }
    pthread_t thread;
    pthread_mutex_lock(&listenerMutex);
    Client_Info * curr_client_info =
        new Client_Info(client_connection_fd, client_id, socket);
    cout << "current client id:" << client_id << endl;
    client_id++;
    pthread_mutex_unlock(&listenerMutex);
    cout << "try to create new thread" << endl;
    int res = pthread_create(&thread, NULL, request_handle, curr_client_info);
    if (res != 0) {
      cout << "Failed to create new thread" << endl;
    }
  }
}
