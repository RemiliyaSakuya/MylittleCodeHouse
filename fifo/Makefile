.PHONY:all

all:server client

server:server.cxx
	g++ -o $@ $^ -std=c++11
client:client.cxx
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -f server client 