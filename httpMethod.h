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
    void keepSending(int server_fd, int client_connection_fd, bool if_cache_reponse, vector<char>full_response, Cache cache_map, string client_request);
    void recvAll(int server_fd,int client_connection_fd,int currLen,int totalLen,bool if_cache_reponse, vector<char>full_response, Cache cache_map, string client_request);
    string addEtag(string full_response,string eTag);
    string addLMDF(string full_response,string lmdf);
};
