#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "cache.h"
#include "response.h"


using namespace std;


class HttpMethod {
    public:
    void connectRequest(int server_fd, int client_connection_fd);
    void getRequest(int server_fd, int client_connection_fd,char buffer[], int length,Cache cache_map);
    void postRequest(int server_fd, int client_connection_fd, char buffer[], int length);
  
};