all: client server

client: client.o
	g++ -o client client.o

client.o: client.cpp
	g++ -c client.cpp

server: server.o
	g++ -pthread -o server server.o

server.o: server.cpp
	g++ -c server.cpp

clean:
	rm -r *.o client server
