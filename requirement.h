#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

using namespace std;
class Requirement {
    public:
    int connect(int client_fd,int server_fd,int id);
    int post(int client_fd,int server_fd,int id);
    int get(int client_fd,int server_fd,int id);
};