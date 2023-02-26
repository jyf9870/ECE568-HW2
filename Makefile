all: server

server: listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp response.cpp
	g++ --std=c++11 -pthread -g -o main listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp response.cpp
.PHONY:
	clean

clean:
	rm -rf *.o *~ *# main