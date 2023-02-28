#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
class Request {
 private:
  string request;
  map<string, string> requestMap;
  string method;
  string methodContent;
  string methodHttp;
  string port;
  string request_line;
  string IP;
  string time;
  string server_hostname;

 public:
  Request(string _request) : request(_request) { readRequest(); }
  void readRequest();
  map<string, string> getRequestMap() const;
  string getMethod() const;
  string getMethodContent() const;
  string getPort() const;
  string getRequesLine() const;
  string getIP() const;
  string getTime() const;
  string get_server_hostname() const;
};