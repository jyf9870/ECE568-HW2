#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
using namespace std;
class Socket {
    public:
    int connectToServer(const char * hostname);
};