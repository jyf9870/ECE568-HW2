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
        if(length2 < 0){
            //add log to log file
            cout<<"400 error"<<endl;

        }
        Response response(recvBuffer, sizeof(recvBuffer));
        send(client_connection_fd,recvBuffer, length2, 0);
        bool if_cache_reponse = false;
        vector<char> full_response;
        if(response.hasPrivate() || response.hasNoStore()){
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
                full_response.insert(full_response.end(), new_recvBuffer, new_recvBuffer + sizeof(new_recvBuffer));
            }
        }
        if(if_cache_reponse){
            cache_map.put(client_request, full_response);
            for (char c : *cache_map.get(client_request)) {
            std::cout << c << "";
            }  
        }
        return;


    }else{
        //find from cache --> revalidate/ if expire/ directly
        return;

    }
}
   

void HttpMethod::postRequest(int server_fd, int client_connection_fd, char buffer[], int length){
    // Parse the HTTP request
    http::request_parser<http::dynamic_body> parser;
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(buffer, length), ec);
    if (ec) {
        std::cerr << "Error parsing HTTP request: " << ec.message() << std::endl;
        return;
    }
    const auto& req = parser.get();

    // Forward the request to the server
    std::ostringstream req_stream;
    req_stream << req;
    std::string req_str = req_stream.str();
    if (send(server_fd, req_str.data(), req_str.size(), 0) == -1) {
        std::cerr << "Error sending request to server: " << strerror(errno) << std::endl;
        return;
    }

    // Receive the response from the server and relay it to the client
    std::array<char, 100000> recv_buf;
    while (true) {
        int n = recv(server_fd, recv_buf.data(), recv_buf.size(), 0);
        if (n == -1) {
            std::cerr << "Error receiving response from server: " << strerror(errno) << std::endl;
            return;
        } else if (n == 0) {
            // Server closed the connection
            return;
        } else {
            if (send(client_connection_fd, recv_buf.data(), n, 0) == -1) {
                std::cerr << "Error relaying response to client: " << strerror(errno) << std::endl;
                return;
            }
        }
    }
}




//-------test boost and reponse------------------------------------- 


    //     std::string httpResponse =
    //    "HTTP/1.1 200 OK\r\n"
    //                  "Cache-Control: private, max-age=3600, must-revalidate\r\n"
    //                  "ETag: \"123456789\"\r\n"
    //                  "Content-Type: text/plain\r\n"
    //                  "Content-Length: 12\r\n"
    //                  "\r\n";

    // // Parse the HTTP response using Response class
    // Response response(httpResponse.data(), httpResponse.size());

    // // Check if the response has chunked encoding
    // assert(response.isChunked() == false);

    // // Get the last modified header
    // boost::beast::string_view lastModified = response.hasLastModified();
    // cout<<lastModified<<endl;

    // // Check if the response has no-store cache control
    // assert(response.hasNoStore() == false);

    // // Check if the response has no-cache cache control
    // assert(response.hasNoCache() == false);
    //  assert(response.hasPrivate() == true);
    //  assert(response.hasMustRevalidate() == true);


    // // Get the ETag header
    // boost::beast::string_view eTag = response.eTag();
    // cout<<eTag<<endl;

    // // Get the max-age value from the cache-control header
    // int maxAge = response.maxAge();
    // cout<<maxAge<<endl;
    // //assert(maxAge == 3600);

    // // Get the full response as a vector of chars
    // std::vector<char> fullResponse = response.getResponse();
    // assert(fullResponse.size() == httpResponse.size());
    // assert(std::equal(fullResponse.begin(), fullResponse.end(), httpResponse.begin()));

    // std::cout << "All tests passed!\n";

