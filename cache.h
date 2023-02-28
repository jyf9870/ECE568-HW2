#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <vector>

using namespace std;
class Cache {
 private:

  unordered_map <string,vector<char>> responseMap;
 public:
  void put(string request, vector<char> response);
  bool contains(string request);
  vector<char>* get(string request);
};