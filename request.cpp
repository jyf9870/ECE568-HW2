#include "request.h"

#include <iostream>
using namespace std;

// fuction that returns a string of time in	UTC, with	a format given by	asctime
string getCurrTime() {
  // Get the current time in UTC
  std::time_t now = std::time(NULL);

  // Convert the time to a tm structure in UTC
  std::tm * tm_utc = std::gmtime(&now);

  // Format the time as a string with asctime format
  string time_str = std::asctime(tm_utc);

  // Output the formatted time string
  return time_str;
}

void Request::readRequest() {
  this->time = getCurrTime();  // initialize field: time
  size_t pos = 0;
  string delimiter = ": ";
  stringstream ss(this->request);
  int lineInd = 0;
  string line;
  int count = 0;
  while (getline(ss, line, '\r')) {
    if (!line.empty()) {
      // remove the '\n' character at the end of the line
      if(lineInd == 0){
        this->request_line = line;
        lineInd++;
      }
      if (line.back() == '\n') {
        line.pop_back();
      }
      // process the line
      if (count == 0) {
        if ((pos = line.find(" ")) != string::npos) {
          method = line.substr(0, pos);
          string temp = line.substr(pos + 1);
          if ((pos = temp.find(" ")) != string::npos) {
            methodContent = temp.substr(0, pos);
            methodHttp = temp.substr(pos + 1);
          }
        }
      }
      else {
        if ((pos = line.find(delimiter)) != string::npos) {
          string name = line.substr(0, pos);
          string body = line.substr(pos + 2);
          if (name == "Host") {
            size_t position = body.find_first_of(":\n");
            if (position != string::npos) {
              string hostname = body.substr(0, position);
              port = body.substr(position + 1);
              requestMap.insert(pair<string, string>(name, hostname));
            }
            else {
              port = "80";
              requestMap.insert(pair<string, string>(name, body));
            }
          }
          else {
            requestMap.insert(pair<string, string>(name, body));
          }
        }
      }
      // skip the '\n' character after the '\r'
      ss.ignore(1);
      count++;
    }
  }

  /* 
    * Only for test --> print all of the request the proxy received from  client
    */
  // for (std::map<string,string>::iterator it=requestMap.begin(); it!=requestMap.end(); ++it){
  //   std::cout << it->first << " => " << it->second << '\n';
  // }

  //   std::cout << method << endl;
  //   std::cout << methodContent << endl;
  //   std::cout << methodHttp << endl;
  //   std::cout << port << endl;
}

map<string, string> Request::getRequestMap() const {
  return requestMap;
}
string Request::getMethod() const {
  return method;
}
string Request::getMethodContent() const {
  return methodContent;
}
string Request::getPort() const {
  return port;
}
string Request::getRequesLine() const {
  return this->request_line;
}
string Request::getIP() const {
  return this->IP;
}
string Request::getTime() const {
  return this->time;
}
