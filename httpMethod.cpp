#include "httpMethod.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile(
    "/var/log/erss/proxy.log");  // If you don't have the erss directory under /var/log, you need to create that directory fisrt.

void addToLog(const std::string & message) {
  pthread_mutex_lock(&myMutex);
  logFile << message << std::endl;
  pthread_mutex_unlock(&myMutex);
}

void addCacheMessage(int clientID, const std::string & message) {
  stringstream cache_message_ss;
  cache_message_ss << clientID << ": " << message;
  addToLog(cache_message_ss.str());
}

// Convert GMT time string to UTC time string in asctime format
std::string convert_expire_time(const std::string & gmt_time_str) {
  std::tm tm = {};
  std::istringstream iss(gmt_time_str);
  iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S %Z");

  std::time_t t = std::mktime(&tm);
  char buffer[30];
  std::strftime(buffer, sizeof(buffer), "%c", std::gmtime(&t));

  return buffer;
}

// funtion that takes a client socket descriptor and returns its IP as a stirng
std::string getClientIP(int client_connection_fd) {
  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);
  if (getpeername(client_connection_fd,
                  (struct sockaddr *)&client_address,
                  &client_address_len) == 0) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
    return std::string(client_ip);
  }
  else {
    // handle error
    return "";
  }
}

void HttpMethod::connectRequest(int server_fd,
                                int client_connection_fd,
                                int clientID,
                                string request_line,
                                string server_hostname) {
  stringstream request_ss;
  request_ss << clientID << ": Requesting \"" << request_line << "\" from " << server_hostname;
  addToLog(request_ss.str());  // ID: Requesting "REQUEST" from SERVER
  send(client_connection_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

  stringstream response_ss;
  response_ss << clientID << ": Received \""
              << "HTTP/1.1 200 OK"
              << "\" from " << server_hostname;
  addToLog(response_ss.str());  // ID: Received "RESPONSE" from	SERVER

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
void HttpMethod::getRequest(int server_fd,
                            int client_connection_fd,
                            char buffer[],
                            int length,
                            Cache & cache_map,
                            int clientID,
                            string request_line,
                            string server_hostname) {
  std::string client_request(buffer);

  //if not in the cache
  cout << cache_map.size() << "outside map" << endl;
  if (!cache_map.contains(client_request)) {
    addCacheMessage(clientID, "not in cache");  // add to log: ID: not in cache

    stringstream request_ss;
    request_ss << clientID << ": Requesting \"" << request_line << "\" from " << server_hostname;
    addToLog(request_ss.str());  // ID: Requesting "REQUEST" from SERVER

    getEntire(server_fd,
              client_connection_fd,
              buffer,
              length,
              cache_map,
              clientID,
              server_hostname);
    cout << cache_map.size() << "inside map" << endl;
  }
  //if in the cache
  else {
    cout << "from cache!!!!" << endl;
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
    if (hasNoCache || (maxAge == -1 && hasMustRevalidate)) {
      //if no eTag and no last-modified,directly send request ro server, like the common get
      if (eTag.empty() && lmdf.empty()) {
        // ADD TO LOG: NEET TO REVALIDNATE
        addCacheMessage(clientID, "in cache, requires validation");
        stringstream request_ss_1;
        request_ss_1 << clientID << ": Requesting \"" << request_line << "\" from "
                     << server_hostname;
        addToLog(request_ss_1.str());  // ID: Requesting "REQUEST" from SERVER
        getEntire(server_fd,
                  client_connection_fd,
                  buffer,
                  length,
                  cache_map,
                  clientID,
                  server_hostname);
      }
      else {
        //has eTag or Last-modified: revalidate
        if (!eTag.empty()) {
          std::string add_etag = "If-None-Match: " + eTag.to_string() + "\r\n";
          client_request = client_request.insert(client_request.length() - 2, add_etag);
        }
        if (!lmdf.empty()) {
          std::string add_lmdf = "If-Modified-Since: " + lmdf.to_string() + "\r\n";
          client_request = client_request.insert(client_request.length() - 2, add_lmdf);
        }
        std::string req_msg_str = client_request;
        const char * req_new_msg = client_request.c_str();
        int send_len;
        if ((send_len = send(server_fd, req_new_msg, req_msg_str.size() + 1, 0)) <
            0) {  // send request with validator
          cout << ":ERROR send validator fails" << endl;
        }
        char new_resp[65536] = {0};
        int new_len = recv(server_fd, &new_resp, sizeof(new_resp), 0);
        if (new_len <= 0) {
          cout << ":ERROR receive validation fails" << endl;
        }
        std::string checknew(new_resp, new_len);
        if (checknew.find("304 Not Modified") != std::string::npos) {  // validate success
          cout << ": NOTE revalidate successfullly" << endl;
          //send from cache

          // ADD TO LOG: VALID
          addCacheMessage(clientID, "in cache, valid");

          sendFromMap(client_connection_fd, *cache_map.get(client_request));
        }
        else {
          // ADD TO LOG: IN CACHE, NEED TO REVALIDATE
          addCacheMessage(clientID, "in cache, requires validation");

          stringstream request_ss_2;
          request_ss_2 << clientID << ": Requesting \"" << request_line << "\" from "
                       << server_hostname;
          addToLog(request_ss_2.str());  // ID: Requesting "REQUEST" from SERVER
          getEntire(server_fd,
                    client_connection_fd,
                    buffer,
                    length,
                    cache_map,
                    clientID,
                    server_hostname);
        }
      }
    }
    else if (maxAge != -1 && hasMustRevalidate) {
      //if expired, revalidate
      string date_str = response.hasDate();
      string expire_str = response.hasExpire();
      //if expire, revalidate
      if (date_str != "" && expire_str != "" &&
          is_expired(date_str, maxAge, expire_str)) {
        // add to log: ID: in cache, but expired at EXPIREDTIME
        stringstream cache_expire;
        cache_expire << "in cache, but expired at " << convert_expire_time(expire_str);
        addCacheMessage(clientID, cache_expire.str());
        // ADD TO LOG: ID: in cache, requires validation
        addCacheMessage(clientID, "in cache, requires validation");
        getEntire(server_fd,
                  client_connection_fd,
                  buffer,
                  length,
                  cache_map,
                  clientID,
                  server_hostname);
      }
      else {
        //if not expire, send from cache

        // add to log: ID: in cache, valid
        addCacheMessage(clientID, "in cache, valid");
        sendFromMap(client_connection_fd, *cache_map.get(client_request));
      }
    }
    else {
      addCacheMessage(clientID, "in cache, valid");  // add to log: ID: in cache, valid
      sendFromMap(client_connection_fd, *cache_map.get(client_request));
    }
  }
}

// buffer[] is the c_string of the request from client, length is the length of the c_string
void HttpMethod::postRequest(int server_fd,
                             int client_connection_fd,
                             char buffer[],
                             int length,
                             int clientID,
                             string request_line,
                             string server_hostname) {
  std::string client_request(buffer);
  int total = requestLength(buffer, length);

  int headerLen = client_request.find("\r\n\r\n") + 4;
  int conLen = length - headerLen;
  while (total > length) {
    char new_server_msg[65536];
    int tempLen = recv(client_connection_fd, new_server_msg, sizeof(new_server_msg), 0);
    if (tempLen <= 0)
      break;
    length += tempLen;
    std::string temp(new_server_msg, tempLen);
    client_request = client_request + temp;
  }

  //ID: Requesting "REQUEST" from SERVER
  stringstream request_ss_1;
  request_ss_1 << clientID << ": Requesting \"" << request_line << "\" from " << server_hostname;
  addToLog(request_ss_1.str());  // ID: Requesting "REQUEST" from SERVER

  if (send(server_fd, client_request.c_str(), length, 0) == -1) {
    std::cerr << "Error sending request to server: " << strerror(errno) << std::endl;
    return;
  }
  char recvBuffer[100000];

  //ID: Received "RESPONSE" from	SERVER

  int n = recv(server_fd, recvBuffer, 100000, 0);
  if (n == -1) {
    std::cerr << "Error receiving response from server: " << strerror(errno) << std::endl;
    return;
  }

  string str(recvBuffer);
  int responseHeader = str.find("\r\n");
  string resp_res;
  if (responseHeader != string::npos) {
    resp_res = str.substr(0, responseHeader);
  }

  stringstream response_ss;
  response_ss << clientID << ": Received \"" << resp_res << "\" from " << server_hostname;
  addToLog(response_ss.str());  // ID: Requesting "REQUEST" from SERVER

  if (send(client_connection_fd, recvBuffer, n, 0) <= 0) {
    std::cerr << "Error relaying response to client: " << strerror(errno) << std::endl;
    return;
  }
  //sometimes go with response content somtimes not????
  const char * end_of_headers = std::strstr(recvBuffer, "{");
  if (end_of_headers != nullptr)
    return;

  Response res(recvBuffer, n);
  int ctl = res.hasContentLength();
  int currLen = 0;
  while (currLen < ctl) {
    char new_recvBuffer[100000];
    int n = recv(server_fd, new_recvBuffer, 100000, 0);

    if (n <= 0)
      break;
    currLen += n;
    if (send(client_connection_fd, recvBuffer, n, 0) <= 0) {
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
      if (if_cache_reponse)
        cache_map.put(client_request, full_response);
    }
    else {
      int currLen = 0;
      while (currLen < hasContentLength) {
        char new_recvBuffer[100000];
        int tempLength = recv(server_fd, new_recvBuffer, 100000, 0);
        if (tempLength <= 0)
          break;
        currLen += tempLength;
        send(client_connection_fd, new_recvBuffer, tempLength, 0);
        if (if_cache_reponse) {
          full_response.insert(full_response.end(),
                               new_recvBuffer,
                               new_recvBuffer + sizeof(new_recvBuffer));
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

const char * HttpMethod::handleMapResponse(vector<char> data) {
  // Convert the vector to a const char* array
  const char * data_ptr = data.data();

  // Find the position of "\r\n\r\n" in the array
  const char * end_of_headers = std::strstr(data_ptr, "\r\n\r\n");

  if (end_of_headers != nullptr) {
    // Calculate the length of the headers
    size_t headers_len = end_of_headers - data_ptr;

    // Allocate a new array for the headers
    char * headers = new char[headers_len + 5];

    // Copy the headers to the new array
    std::memcpy(headers, data_ptr, headers_len + 4);
    headers[headers_len] = '\0';
    return headers;
  }
  return data_ptr;
}

void HttpMethod::getEntire(int server_fd,
                           int client_connection_fd,
                           char buffer[],
                           int length,
                           Cache & cache_map,
                           int clientID,
                           string server_hostname) {
  //send request ro server

  // ID: Requesting "REQUEST" from SERVER
  send(server_fd, buffer, length, 0);
  char recvBuffer[100000];

  // ID: Received "RESPONSE" from	SERVER
  int length2 = recv(server_fd, recvBuffer, 100000, 0);

  if (length2 < 0) {
    //add log to log file
    cout << "400 error" << endl;
  }

  string str(recvBuffer);
  int responseHeader = str.find("\r\n");
  string resp_res;
  if (responseHeader != string::npos) {
    resp_res = str.substr(0, responseHeader);
  }

  stringstream response_ss;
  response_ss << clientID << ": Received \"" << resp_res << "\" from " << server_hostname;
  addToLog(response_ss.str());  // ID: Requesting "REQUEST" from SERVER

  Response response(recvBuffer, sizeof(recvBuffer));
  send(client_connection_fd, recvBuffer, length2, 0);

  //init
  bool if_cache_reponse = false;
  vector<char> full_response = response.getResponse();
  bool isChunked = response.isChunked();
  int hasContentLength = response.hasContentLength();
  string client_request(buffer);

  //do not need to cache
  if (response.hasPrivate() || response.hasNoStore()) {
    //don't cache, directly get
    recvResponse(server_fd,
                 client_connection_fd,
                 length2,
                 if_cache_reponse,
                 full_response,
                 cache_map,
                 client_request,
                 isChunked,
                 hasContentLength);
  }
  else {
    //need to cache, add to cache
    if_cache_reponse = true;
    recvResponse(server_fd,
                 client_connection_fd,
                 length2,
                 if_cache_reponse,
                 full_response,
                 cache_map,
                 client_request,
                 isChunked,
                 hasContentLength);
  }
}

void HttpMethod::sendFromMap(int client_connection_fd, vector<char> response) {
  char * response_data = response.data();
  size_t response_size = response.size();
  int slen = send(client_connection_fd, response_data, response_size, 0);
  if (slen == -1) {
    cout << ": ERROR failure in sending response from proxy to client" << endl;
    return;
  }
}

bool HttpMethod::is_expired(const std::string & date_str,
                            int max_age,
                            const std::string & expires_str) {
  // Parse the "Date" and "Expires" headers
  boost::posix_time::ptime date =
      boost::date_time::parse_delimited_time<boost::posix_time::ptime>(date_str, ' ');
  boost::posix_time::ptime expires =
      boost::date_time::parse_delimited_time<boost::posix_time::ptime>(expires_str, ' ');

  // Calculate the expiration time based on the "max-age" header
  boost::posix_time::ptime max_age_expiration =
      date + boost::posix_time::seconds(max_age);

  if (max_age_expiration > expires) {
    return true;
  }
  else {
    return false;
  }
}

int HttpMethod::requestLength(char * server_msg, int mes_len) {
  std::string msg(server_msg, mes_len);
  size_t pos = msg.find("Content-Length: ");
  if (pos != std::string::npos) {
    size_t head_end = msg.find("\r\n", pos);
    if (head_end != std::string::npos) {
      return std::stoi(std::string(msg.substr(pos + 16, head_end)));
    }
  }
  return -1;
}

// int main() {
//   std::string date_str = "Sat, 28 Feb 2023 15:30:00 GMT";
//   int max_age = 3600;
//   std::string expires_str = "Sun, 01 Mar 2023 15:30:00 GMT";
//   bool expired = is_expired(date_str, max_age, expires_str);
//   std::cout << "Is expired: " << std::boolalpha << expired << std::endl;

//   return 0;
// }
