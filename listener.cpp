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

#include "httpMethod.h"
#include "request.h"
#include "socket.h"

using namespace std;

class Client_Info {
 public:
  int client_connection_fd;
  int client_id;
  Socket socket;
  Cache cache_map;
};

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile(
    "/var/log/erss/proxy.log");  // If you don't have the erss directory under /var/log, you need to create that directory fisrt.

void addToLog(std::ofstream & logFile, const std::string & message) {
  pthread_mutex_lock(&myMutex);
  logFile << message << std::endl;
  pthread_mutex_unlock(&myMutex);
}

// funtion that takes a client socket descriptor and returns its IP as a stirng
std::string getClientIP(int client_connection_fd) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    if (getpeername(client_connection_fd, (struct sockaddr*)&client_address, &client_address_len) == 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        return std::string(client_ip);
    } else {
        // handle error
        return "";
    }
}

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
    const char * req400 = "HTTP/1.1 400 Bad Request";
    //TODO!!!: write something into the log file!
    return NULL;
  }

  stringstream request_ss;
  request_ss << curr_client_info->client_id << ": " << ctopRequest->getRequesLine() << " from " << getClientIP(client_connection_fd) << " @ " << ctopRequest->getTime();
  addToLog(logFile, request_ss.str());  // add to log: ID: "REQUEST" from IPFROM @ TIME

  string hostname = ctopRequest->getRequestMap().find("Host")->second;
  int server_fd = curr_client_info->socket.connectToServer(
      hostname.c_str(),
      ctopRequest->getPort()
          .c_str());  //connected successfully from client to proxy and proxy to server

  HttpMethod httpMethod;
  if (ctopRequest->getMethod() == "CONNECT") {
    cout << "I am handling connect TAT!!!!!!" << endl;
    httpMethod.connectRequest(server_fd, client_connection_fd);
    cout << "CONNECTCONNECTCONNECTCONNECT!!!!!!" << endl;
    return NULL;
  }

  if (ctopRequest->getMethod() == "GET") {
    cout << "I am handling get TAT!!!!!!" << endl;
    httpMethod.getRequest(
        server_fd, client_connection_fd, buffer, length, curr_client_info->cache_map);
    cout << "getgetgetget!!!!!!" << endl;
    return NULL;
  }

  if (ctopRequest->getMethod() == "POST") {
    cout << "I am handling post TAT!!!!!!" << endl;
    httpMethod.postRequest(server_fd, client_connection_fd, buffer, length);
    cout << "postpostpostpost!!!!!!" << endl;
    return NULL;
  }

  close(server_fd);
  close(client_connection_fd);
  return NULL;
}

int main(int argc, char * argv[]) {
  Socket socket;
  Cache cache_map;
  int socket_fd = socket.connectToClient();  //connect to client
  if (socket_fd == -1) {
    const string message = "Proxxy can not create server socket for the client!";
    cout << message << endl;
    addToLog(logFile, message);
    return -1;
  }
  int client_id = 0;
  while (true) {
    cout << "Waiting for connection on port " << socket.port << endl;
    int client_connection_fd = socket.acceptToClient(socket_fd);  //accepted

    if (client_connection_fd == -1) {
      pthread_mutex_lock(&myMutex);
      cout << "can not connect client" << endl;
      logFile << "(no-id): ERROR in connecting client" << std::endl;
      pthread_mutex_unlock(&myMutex);
      continue;
    }
    pthread_t thread;
    pthread_mutex_lock(&myMutex);
    Client_Info * curr_client_info = new Client_Info();
    curr_client_info->client_connection_fd = client_connection_fd;
    curr_client_info->client_id = client_id;
    curr_client_info->cache_map = cache_map;
    curr_client_info->socket = socket;
    cout << "current client id:" << client_id << endl;
    client_id++;
    pthread_mutex_unlock(&myMutex);
    cout << "try to create new thread" << endl;
    int res = pthread_create(&thread, NULL, request_handle, curr_client_info);
    if (res != 0) {
      cout << "Failed to create new thread" << endl;
    }
  }
}
