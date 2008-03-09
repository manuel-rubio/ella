all: libhello.so.1.0.0 usa_hello usa_hello_dyn

libhello.so.1.0.0: hello.o
	gcc -shared -Wl,-soname,libhello.so.1 -o libhello.so.1 hello.o -lc

hello.o: hello.h hello.c
	gcc -c -fPIC hello.c

usa_hello: usa_hello.c
	gcc usa_hello.c libhello.so.1 -o usa_hello

usa_hello_dyn: usa_hello_dyn.c
	gcc usa_hello_dyn.c -ldl -o usa_hello_dyn

clean:
	rm -f libhello.so.1 hello.o
	rm -f usa_hello
	rm -f usa_hello_dyn
