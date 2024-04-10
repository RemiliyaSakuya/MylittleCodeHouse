.PHONY:all
all:udp_client udp_server

udp_client: udp_client.cc
	g++ -o $@ $^ -std=c++11 -lpthread
udp_server: udp_server.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -rf udp_client udp_server
