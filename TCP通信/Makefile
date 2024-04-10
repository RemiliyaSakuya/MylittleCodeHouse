.PHONY:all
all:tcp_client tcp_server

tcp_client:tcp_client.cc
	g++ -o $@ $^ -std=c++11 -lpthread
tcp_server:tcp_server.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -rf tcp_client tcp_server