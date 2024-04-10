.PHONY:all
all:shmServer shmClient
shmServer:shmServer.c++
	g++ -o $@ $^ -std=c++11
shmClient:shmClient.c++
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -f shmClient shmServer