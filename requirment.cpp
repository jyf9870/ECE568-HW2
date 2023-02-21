#include "requirement.h"

using namespace std;

int Requirement::connect(int client_fd,int server_fd,int id){
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);

    select(nfds, &readfds, NULL, NULL, NULL);
    int fd[2] = {server_fd, client_fd};
    int len;
    for (int i = 0; i < 2; i++) {
      char message[65536] = {0};
      if (FD_ISSET(fd[i], &readfds)) {
        len = recv(fd[i], message, sizeof(message), 0);
        if (len <= 0) {
          return;
        }
        else {
          if (send(fd[1 - i], message, len, 0) <= 0) {
            return;
          }
        }
      }
    }
  }

}

int Requirement::get(int client_fd,int server_fd,int id){


}

int Requirement::post(int client_fd,int server_fd,int id){


}