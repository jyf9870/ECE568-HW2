all: server

server: listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp
	# g++ --std=c++11 -g -o main listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp -lpthread
	g++ --std=c++11 -pthread -g -o main listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp
.PHONY:
	clean

clean:
	rm -rf *.o *~ *# main