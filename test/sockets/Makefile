all: client  resolv  server

client: client.c
	gcc -o client client.c

resolv: resolv.c
	gcc -o resolv resolv.c

server: server.c
	gcc -o server server.c

clean:
	rm -f client  resolv  server

