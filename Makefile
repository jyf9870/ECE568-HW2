all: server

server: listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp response.cpp
	g++ --std=gnu++11 -pthread -g -o main listener.cpp request.cpp socket.cpp httpMethod.cpp cache.cpp response.cpp -lboost_date_time
.PHONY:
	clean

clean:
	rm -rf *.o *~ *# main