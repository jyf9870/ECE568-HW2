#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <ctime>
#include <sstream>
#include <vector>

using namespace std;
class Request {
 private:
  string request;
  map <string,string> requestMap;
  string method;
  string methodContent;
  string methodHttp;
  string port;
  string request_line;
  string IP;
  string time;
  
 public:
  Request(string request) : request(request) {
    readRequest();
  }
  void readRequest();
  map<string, string> getRequestMap() const;
  string getMethod() const;
  string getMethodContent() const;
  string getPort() const;
  string getRequesLine() const;
  string getIP() const;
  string getTime() const;
};