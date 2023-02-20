#include "request.h"
#include <cstring>
#include <exception>
#include <iostream>
using namespace std;


void Request::readLine() {
  size_t pos = request.find_first_of("\r\n");
  string s = request.substr(0, pos);
}

void Request::readRequest() {
}