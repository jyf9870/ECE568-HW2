all: server

server: server.cpp request.cpp socket.cpp
	g++ --std=c++11 -g -o server server.cpp request.cpp socket.cpp 

.PHONY:
	clean

clean:
	rm -rf *.o *~ *# server