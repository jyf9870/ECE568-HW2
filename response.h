#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <functional>
#include <iostream>
#include <vector>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <unistd.h>


#include <stdio.h>
#include <string.h>

// #include <time.h>
// #include "mytime.h"
namespace http = boost::beast::http;
using namespace std;
class Response {
    private:
    const char* res;
    size_t size;
    http::response_parser<http::dynamic_body> parser;
    bool has_cache_control = false;
    boost::beast::string_view cacheControl;

    // MyTime expire_time;
    // MyTime response_time;
    // bool must_revalidate;
    // two validators
 public:
      Response(const char* res, size_t size):res(res), size(size) {
        parseHttpResponse(res,size);
      }

    // exp_str(""),
    // must_revalidate(false){}

    void parseHttpResponse(const char* res, size_t size);
    bool isChunked();
    boost::beast::string_view hasLastModified();
    bool hasNoStore();
    bool hasNoCache();
    boost::beast::string_view eTag();
    int maxAge();
    bool hasPrivate();
    bool hasMustRevalidate();
    int hasContentLength();
    std::vector<char> getResponse();
    std::vector<char> addToMap(std::vector<char> full_response, char * following_reponse);
    string toStr();
    string hasExpire();
    string hasDate();
 
};