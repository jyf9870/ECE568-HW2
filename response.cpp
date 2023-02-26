#include "response.h"


void Response::parseHttpResponse(const char* res, size_t size) {
    boost::beast::flat_buffer buffer;
    // Parse the HTTP response
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(res, size), ec);

    // Check for errors
    if (ec) {
        std::cout << "Error parsing HTTP response: " << ec.message() << std::endl;
        return;
    }else{
        std::cout << "succeed"<< std::endl;
    }
    auto iter = parser.get().find("Cache-Control");
    if (iter != parser.get().end()) {
        cacheControl = iter->value();
        has_cache_control = true;
    }else{
        has_cache_control = false; 
    }
    return;
}


bool Response::isChunked(){
    // Check if the response has a chunk
    if (parser.get().chunked()) {
        return true;
    }
    return false;
}

boost::beast::string_view Response::hasLastModified(){
    // Get the last modified header
    auto iter2 = parser.get().find("Last-Modified");
    if (iter2 != parser.get().end()) {
        return iter2->value();
    } else {
        return boost::beast::string_view();
    }
    //use below statement to compare
    //boost::beast::string_view myString = myResponse.hasLastModified(myParser);
    //if (myString.empty()) {
    
}

bool Response::hasNoStore(){
    if(has_cache_control){
        if (cacheControl.find("no-store") != boost::beast::string_view::npos) {
            // Cache-Control header contains no-store directive
            return true;
        }
    }
    return false;
}

bool Response::hasNoCache(){
    if(has_cache_control){
        if (cacheControl.find("no-cache") != boost::beast::string_view::npos) {
            // Cache-Control header contains no-cache directive
            return true;
        }
    }
    return false;
}

boost::beast::string_view Response::eTag(){
    // Get the ETag header
    auto iter3 = parser.get().find("ETag");
    if (iter3 != parser.get().end()) {
        boost::beast::string_view eTag = iter3->value();
        // eTag contains the ETag header value
        return eTag;
    }
    else{
        return boost::beast::string_view();
    }
}


int Response::maxAge(){
    // Get the Cache-Control header
   
    if (has_cache_control) {
        // Find the max-age directive
        size_t pos = cacheControl.find("max-age=");
        if (pos != boost::beast::string_view::npos) {
            // Extract the value of the max-age directive
            size_t pos2 = cacheControl.find(",",pos+8);
            cout<<"pos2"<<endl;
            cout<<pos2<<endl;
            int maxAge = -1;
            if(pos2 != boost::beast::string_view::npos) {
                 maxAge= std::stoi(std::string(cacheControl.substr(pos+8,pos2)));
            }else{
                 maxAge= std::stoi(std::string(cacheControl.substr(pos+8, cacheControl.size())));
            }
                 // maxAge contains the max-age value
            return maxAge;           
        }
    }
    // max-age directive not found or invalid, return a default value
    return -1;
}

bool Response::hasPrivate(){
   if(has_cache_control){
        if (cacheControl.find("private") != boost::beast::string_view::npos) {
            return true;
        }
    }
    return false;
}

bool Response::hasMustRevalidate(){
    if(has_cache_control){
        if (cacheControl.find("must-revalidate") != boost::beast::string_view::npos) {
            return true;
        }
    }
    return false;
}

std::vector<char> Response::getResponse(){
    std::vector<char> full_response(res, res + size);
    return full_response;
}

std::vector<char> Response::addToMap(std::vector<char> full_response, char * following_reponse){
    full_response.insert(full_response.end(), following_reponse, following_reponse + sizeof(following_reponse));
    return full_response;
}
