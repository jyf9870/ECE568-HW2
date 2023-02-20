#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

class Request {
 public:
  std::string request;
  std::string method;
  std::string fileName;
  std::map <std::string,std::string> map;
 public:
  Request(std::string request) : request(request) {
    readLine();
    readRequest();
  }
  void readLine();
  void readRequest();
};