 #include "cache.h"
 #include <iostream> 
 #include <map> 
 
void Cache::put(string request, vector<char> response){
    responseMap.insert(std::make_pair(request, response));
    return;
}
bool Cache::contains(string request){
    if (responseMap.count(request) > 0) {
        return true;
    } else {
        return false;
    }
}

vector<char>* Cache::get(string request) {
    if (contains(request)) {
        return &responseMap[request];
    } else {
        return nullptr;
    }
}  

