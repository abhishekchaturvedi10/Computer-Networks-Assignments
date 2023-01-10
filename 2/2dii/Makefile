compile: server client

runserver: server
	./server

runclient: client
	./client

server: server.c
	gcc server.c -o server

client: client.c
	gcc client.c -lpthread -o client
