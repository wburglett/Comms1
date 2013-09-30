all:
	gcc Client.c -o Client
	gcc Server.c -o Server

client:
	gcc Client.c -o Client

server:
	gcc Server.c -o Server

clean:
	rm Server.o Client.o Server Client a.out
