#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

#include "cache.h"
#include "response.h"

using namespace std;

class HttpMethod {
 public:
  void connectRequest(int server_fd,
                      int client_connection_fd,
                      int clientID,
                      string request_line,
                      string server_host_name);
  void getRequest(int server_fd,
                  int client_connection_fd,
                  char buffer[],
                  int length,
                  Cache & cache_map,
                  int clientID,
                  string request_line,
                  string server_host_name);
  void postRequest(int server_fd,
                   int client_connection_fd,
                   char buffer[],
                   int length,
                   int clientID,
                   string request_line,
                   string server_hostname);
  void recvResponse(int server_fd,
                    int client_connection_fd,
                    int currLen,
                    bool if_cache_reponse,
                    vector<char> full_response,
                    Cache & cache_map,
                    string client_request,
                    bool isChunked,
                    int hasContentLength);
  const char * handleMapResponse(vector<char> data);
  void getEntire(int server_fd,
                 int client_connection_fd,
                 char buffer[],
                 int length,
                 Cache & cache_map,
                 int clientID,
                 string server_hostname);
  void sendFromMap(int client_connection_fd, vector<char> response);
  bool is_expired(const std::string & date_str,
                  int max_age,
                  const std::string & expires_str);
  int requestLength(char * server_msg, int mes_len);

  void respond502(int client_connection_fd);
};

void addToLog(const std::string & message);

// funtion that takes a client socket descriptor and returns its IP as a stirng
std::string getClientIP(int client_connection_fd);

// Convert GMT time string to UTC time string in asctime format
std::string convert_expire_time(const std::string & gmt_time_str);
