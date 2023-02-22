#include "request.h"
using namespace std;

void Request::readRequest() {
    size_t pos = 0;
    string delimiter = ": ";
    stringstream ss(request);
    string line;
    int count = 0;
    while (getline(ss, line, '\r')) {
      if (!line.empty()) {
          // remove the '\n' character at the end of the line
          if (line.back() == '\n') {
              line.pop_back();
          }
          // process the line
          if(count == 0){
            if((pos = line.find(" ")) != string::npos) {
              method = line.substr(0, pos);
              string temp = line.substr(pos+1);
              if((pos = temp.find(" ")) != string::npos) {
                methodContent = temp.substr(0, pos);
                methodHttp = temp.substr(pos+1);
              }
            }
          }
          else{
            if((pos = line.find(delimiter)) != string::npos) {
                string name = line.substr(0, pos);
                string body = line.substr(pos+2);
                if(name.compare("Host")==0){
                  size_t position = body.find(":");
                  string hostname = body.substr(0, position);
                  requestMap.insert(pair<string,string>(name,hostname));
                  if(position != string::npos){
                    port = body.substr(position+1);
                  }else{
                    port = "80";
                  }
                }else{
                requestMap.insert(pair<string,string>(name,body));
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
    // for (std::map<string,string>::iterator it=requestMap.begin(); it!=requestMap.end(); ++it)
    // std::cout << it->first << " => " << it->second << '\n';
    // std::cout << method << endl;
    // std::cout << methodContent << endl;
    // std::cout << methodHttp << endl;
  }

map<string, string> Request::getRequestMap() const {
  return requestMap;
}
string Request::getMethod() const{
  return method;
}
string Request::getMethodContent() const{
  return methodContent;
}
string Request::getPort() const{
  return port;
}