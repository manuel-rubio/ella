LIBS=`xml2-config --libs`
CFLAGS=`xml2-config --cflags`

all: tree1 xpath1

tree1: tree1.c
	gcc -o tree1 ${LIBS} ${CFLAGS} tree1.c

xpath1: xpath1.c
	gcc -o xpath1 ${LIBS} ${CFLAGS} xpath1.c

clean:
	rm -f tree1 xpath1
