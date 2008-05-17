all: console_cli console_srv

console_cli: console_cli.c
	gcc -o console_cli console_cli.c

console_srv: console_srv.c
	gcc -pthread -o console_srv console_srv.c

clean:
	rm -f console_cli console_srv echo_socket
