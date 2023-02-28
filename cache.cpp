#include "cache.h"

#include <iostream>
#include <unordered_map>
std::mutex cache_mutex;
void Cache::put(string request, vector<char> response) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  responseMap.insert(std::make_pair(request, response));
  return;
}
bool Cache::contains(string request) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  if (responseMap.count(request) > 0) {
    return true;
  }
  else {
    return false;
  }
}

vector<char> * Cache::get(string request) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  if (contains(request)) {
    return &responseMap[request];
  }
  else {
    return nullptr;
  }
}
