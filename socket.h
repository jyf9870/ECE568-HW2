#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
using namespace std;
class Socket {
    public:
    int connectToServer(const char * hostname, const char * port);
    int acceptToClient(int socket_fd);
    int connectToClient();
    const char *port;

};