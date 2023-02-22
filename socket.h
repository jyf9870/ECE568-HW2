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
    void connectRequest(int server_fd, int client_connection_fd);
    const char *port;

};