#include "httpMethod.h"

void HttpMethod::connectRequest(int server_fd, int client_connection_fd) {
  send(client_connection_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

  while (true) {
    fd_set read_fd;
    int selector = server_fd > client_connection_fd ? server_fd : client_connection_fd;

    FD_ZERO(&read_fd);
    FD_SET(server_fd, &read_fd);
    FD_SET(client_connection_fd, &read_fd);
    int res = select(selector + 1, &read_fd, NULL, NULL, NULL);
    if (res <= 0)
      return;

    int fd[2] = {server_fd, client_connection_fd};
    int length;
    for (int i = 0; i < 2; i++) {
      if (FD_ISSET(fd[i], &read_fd)) {
        int ll = i == 0 ? 100000 : 65536;
        char message[ll];
        length = recv(fd[i], message, sizeof(message), 0);
        if (length > 0) {
          if (send(fd[1 - i], message, length, 0) <= 0)
            return;
        }
        else {
          return;
        }
      }
    }
  }
}
void HttpMethod::getRequest(int server_fd, int client_connection_fd, char buffer[], int length, Cache & cache_map){
    std::string client_request(buffer);
    
    //if not in the cache
    if(!cache_map.contains(client_request)){
      getEntire(server_fd,client_connection_fd,buffer,cache_map);
    }
    //if in the cache
    else{
        //get from map
        const char * map_response = handleMapResponse(*cache_map.get(client_request));
        Response response(map_response, sizeof(map_response)); 

        //init
        bool hasMustRevalidate = response.hasMustRevalidate();
        bool hasNoCache = response.hasNoCache();
        int maxAge = response.maxAge();
        boost::beast::string_view eTag = response.eTag();
        boost::beast::string_view lmdf = response.hasLastModified();

        //if has no-cache or only must-revalidate
        if(hasNoCache || (maxAge == -1 && hasMustRevalidate)){
          //if no eTag and no last-modified,directly send request ro server, like the common get
          if(eTag.empty() && lmdf.empty()){
             getEntire(server_fd,client_connection_fd,buffer,cache_map);
          }
          else{
            //has eTag or Last-modified: revalidate
            if(!eTag.empty()){
              std::string add_etag = "If-None-Match: " + eTag.to_string() + "\r\n";
              client_request = client_request.insert(client_request.length() - 2, add_etag);
            }
            if(!lmdf.empty()){
              std::string add_lmdf = "If-Modified-Since: " + lmdf.to_string() + "\r\n";
              client_request = client_request.insert(client_request.length() - 2, add_lmdf);
            }
            std::string req_msg_str = client_request;
            const char * req_new_msg = client_request.c_str();
            int send_len;
            if ((send_len = send(server_fd, req_new_msg, req_msg_str.size() + 1, 0)) < 0) {  // send request with validator
              cout<<":ERROR send validator fails"<<endl;

            }
            char new_resp[65536] = {0};
            int new_len = recv(server_fd, &new_resp, sizeof(new_resp), 0);
            if (new_len <= 0) {
              cout<< ":ERROR receive validation fails"<<endl;
            }
            std::string checknew(new_resp, new_len);
            if (checknew.find("304 Not Modified") != std::string::npos) {  // validate success
              cout<< ": NOTE revalidate successfullly"<<endl;
              //send from cache
              sendFromMap(client_connection_fd, *cache_map.get(client_request));
            }else{
               getEntire(server_fd,client_connection_fd,buffer,cache_map);
            }
          }
        }
        else if(maxAge != -1 && hasMustRevalidate){
          //if expired, revalidate
          if(1){
//checkExpired(int maxAge)
          }else{
            //if not expire, send from cache
            sendFromMap(client_connection_fd, *cache_map.get(client_request));
          }
            
        }else{
          sendFromMap(client_connection_fd, *cache_map.get(client_request));
        }
      }

        //send to server


    }
        

// buffer[] is the c_string of the request from client, length is the length of the c_string
void HttpMethod::postRequest(int server_fd,
                             int client_connection_fd,
                             char buffer[],
                             int length) {
  std::string client_request(buffer);

  // Forward the request to the server
  if (send(server_fd, buffer, length, 0) == -1) {
    std::cerr << "Error sending request to server: " << strerror(errno) << std::endl;
    return;
  }

  // Receive the response from the server and relay it to the client
  char recvBuffer[100000];

  //while (true) {
  int n = recv(server_fd, recvBuffer, 100000, 0);
  cout << n << endl;
  if (n == -1) {
    std::cerr << "Error receiving response from server: " << strerror(errno) << std::endl;
    return;
  }
  else if (n == 0) {
    // Server closed the connection
    return;
  }
  else {
    int length = send(client_connection_fd, recvBuffer, n, 0);
    //    Response response(recvBuffer, sizeof(recvBuffer));
    if (length <= 0) {
      std::cerr << "Error relaying response to client: " << strerror(errno) << std::endl;
      return;
    }
  }
  return;
}

void HttpMethod::recvResponse(int server_fd,
                              int client_connection_fd,
                              int length2,
                              bool if_cache_reponse,
                              vector<char> full_response,
                              Cache & cache_map,
                              string client_request,
                              bool isChunked,
                              int hasContentLength) {
  if (isChunked) {
    while (true) {
      char new_recvBuffer[100000];
      int length3 = recv(server_fd, new_recvBuffer, 100000, 0);
      if (length3 <= 0)
        break;
      send(client_connection_fd, new_recvBuffer, length3, 0);
      if (if_cache_reponse) {
        full_response.insert(
            full_response.end(), new_recvBuffer, new_recvBuffer + sizeof(new_recvBuffer));
      }
    }
    if (if_cache_reponse) {
      cache_map.put(client_request, full_response);
    }
    return;
  }
  else {
    if (hasContentLength == -1) {
      if (if_cache_reponse) cache_map.put(client_request, full_response);
    }
    else {
      while (length2 < hasContentLength) {
        char new_recvBuffer[100000];
        int tempLength = recv(server_fd, new_recvBuffer, 100000, 0);
        length2 += tempLength;
        send(client_connection_fd, new_recvBuffer, tempLength, 0);
        if (if_cache_reponse) {
          full_response.insert(full_response.end(), new_recvBuffer, new_recvBuffer + sizeof(new_recvBuffer));
        }
      }
      if (if_cache_reponse) {
        cache_map.put(client_request, full_response);
      }
      return;
    }
  }
  return;
}

const char * HttpMethod::handleMapResponse(vector<char> data){

   // Convert the vector to a const char* array
    const char* data_ptr = data.data();

    // Find the position of "\r\n\r\n" in the array
    const char* end_of_headers = std::strstr(data_ptr, "\r\n\r\n");

    if (end_of_headers != nullptr) {
        // Calculate the length of the headers
        size_t headers_len = end_of_headers - data_ptr;

        // Allocate a new array for the headers
        char* headers = new char[headers_len + 5];

        // Copy the headers to the new array
        std::memcpy(headers, data_ptr, headers_len + 4);
        headers[headers_len] = '\0';
        return headers;
    }
}
                   

void HttpMethod::getEntire(int server_fd, int client_connection_fd, char buffer[], Cache & cache_map ){
    //send request ro server
  send(server_fd, buffer,sizeof(buffer),0); 
  char recvBuffer[100000];
  int length2 = recv(server_fd, recvBuffer, 100000, 0);
  if(length2 < 0){
      //add log to log file
      cout<<"400 error"<<endl;
  }     
  Response response(recvBuffer, sizeof(recvBuffer)); 
  send(client_connection_fd,recvBuffer, length2, 0);

  //init
  bool if_cache_reponse = false;
  vector<char> full_response = response.getResponse(); 
  bool isChunked = response.isChunked();
  int hasContentLength = response.hasContentLength();
  string client_request(buffer);


  //do not need to cache
  if(response.hasPrivate() || response.hasNoStore()){
  //don't cache, directly get
      recvResponse(server_fd,client_connection_fd,length2,if_cache_reponse,full_response, cache_map, client_request,isChunked, hasContentLength);
  }else{
    //need to cache, add to cache
      if_cache_reponse = true; 
      recvResponse(server_fd,client_connection_fd,length2,if_cache_reponse,full_response, cache_map, client_request,isChunked, hasContentLength);      
  }
}

void HttpMethod::sendFromMap(int client_connection_fd, vector<char> response){
  char* response_data = response.data();
  size_t response_size = response.size();
  int slen = send(client_connection_fd, response_data, response_size, 0);
  if (slen == -1) {
    cout<< ": ERROR failure in sending response from proxy to client"<<endl;
    return;
  }
}

void HttpMethod::checkExpired(){


}