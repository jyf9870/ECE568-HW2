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
    void getRequest(int server_fd, int client_connection_fd,char buffer[], int length,Cache & cache_map);
    void postRequest(int server_fd, int client_connection_fd, char buffer[], int length);
    void recvResponse(int server_fd,int client_connection_fd,int currLen,bool if_cache_reponse, vector<char>full_response, Cache & cache_map, string client_request,bool isChunked, int hasContentLength);
    const char * handleMapResponse(vector<char> data);
    void getEntire(int server_fd, int client_connection_fd, char buffer[], int length,Cache & cache_map );
    void sendFromMap(int client_connection_fd, vector<char> response);
    bool is_expired(const std::string& date_str, int max_age, const std::string& expires_str);
    int requestLength(char * server_msg, int mes_len); 
 
};
