server:
	gcc -o serv server.c
client:
	gcc -o cli client.c

all: server client

