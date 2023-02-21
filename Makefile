all: server

server: listener.cpp request.cpp socket.cpp 
	g++ --std=c++11 -g -o main listener.cpp request.cpp socket.cpp 

.PHONY:
	clean

clean:
	rm -rf *.o *~ *# main