#include "httpMethod.h"

void HttpMethod::connectRequest(int server_fd, int client_connection_fd){
    send(client_connection_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

    while (true) {
        fd_set read_fd;
        int selector = server_fd > client_connection_fd ? server_fd: client_connection_fd;

        FD_ZERO(&read_fd);
        FD_SET(server_fd, &read_fd);
        FD_SET(client_connection_fd, &read_fd);
        int res = select(selector  + 1, &read_fd, NULL, NULL, NULL);
        if (res <= 0) return;
        
        int fd[2] = {server_fd, client_connection_fd};
        int length;
        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(fd[i], &read_fd)) {
                int ll = i == 0 ? 100000 : 65536;
                char message[ll];
                length = recv(fd[i], message, sizeof(message), 0);
                if (length > 0) {
                    if (send(fd[1 - i], message, length, 0) <= 0) return;     
                }
            else{
                return;
                }
            }
        }
    }
}
void HttpMethod::getRequest(int server_fd, int client_connection_fd, char buffer[], int length, Cache cache_map){
    
    
    std::string client_request(buffer);
    if(!cache_map.contains(client_request)){
        send(server_fd, buffer,length,0); 
        char recvBuffer[100000];
        int length2 = recv(server_fd, recvBuffer, 100000, 0);
        Response response(recvBuffer, sizeof(recvBuffer));
        send(client_connection_fd,recvBuffer, length2, 0);
        bool if_cache_reponse = false;
        vector<char> full_response;
        if(response.isChunked() || response.hasNoStore()){
            //do nothing
        }else{
            full_response = response.getResponse();
            if_cache_reponse = true;
        }
        while(length2 > 0){
            char new_recvBuffer[100000];
            length2 = recv(server_fd, new_recvBuffer, 100000, 0);
            if(length2 <= 0) break;
            send(client_connection_fd,new_recvBuffer, length2, 0);
            if(if_cache_reponse){
                full_response = response.addToMap(full_response,new_recvBuffer);
                cache_map.put(client_request, response.getResponse());
            }
        }
       
        for (char c : client_request) {
        std::cout << c << "";
    }
        return;
    }else{
        //find from cache --> revalidate/ if expire/ directly

    }
}
   


void HttpMethod::postRequest(int server_fd, int client_connection_fd){

    return;
}




//-------test boost and reponse------------------------------------- 
//   std::string httpResponse =
//         "HTTP/1.1 200 OK\r\n"
//         "Transfer-Encoding: chunked\r\n"
//         "Last-Modified: Wed, 23 Feb 2023 12:34:56 GMT\r\n"
//         "Cache-Control: no-cache, no-store, max-age=3600\r\n"
//         "\r\n"
//         "5\r\n"
//         "Hello\r\n"
//         "6\r\n"
//         " world\r\n"
//         "0\r\n"
//         "\r\n";

//     // Parse the HTTP response using Response class
//     Response response(httpResponse.data(), httpResponse.size());

//     // Check if the response has chunked encoding
//     assert(response.isChunked() == true);

//     // Get the last modified header
//     boost::beast::string_view lastModified = response.hasLastModified();
//     cout<<lastModified<<endl;

//     // Check if the response has no-store cache control
//     assert(response.hasNoStore() == true);

//     // Check if the response has no-cache cache control
//     assert(response.hasNoCache() == true);

//     // Get the ETag header
//     boost::beast::string_view eTag = response.eTag();
//     assert(eTag.empty() == true);

//     // Get the max-age value from the cache-control header
//     int maxAge = response.maxAge();
//     cout<<maxAge<<endl;
//     assert(maxAge == 3600);

//     // Get the full response as a vector of chars
//     std::vector<char> fullResponse = response.getResponse();
//     assert(fullResponse.size() == httpResponse.size());
//     assert(std::equal(fullResponse.begin(), fullResponse.end(), httpResponse.begin()));

//     std::cout << "All tests passed!\n";

