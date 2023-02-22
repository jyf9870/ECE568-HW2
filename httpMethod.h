#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

class HttpMethod {
    public:
    void connectRequest(int server_fd, int client_connection_fd);
    void getRequest(int server_fd, int client_connection_fd);
    void postRequest(int server_fd, int client_connection_fd);

};