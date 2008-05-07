all: $(MODULES) ../libews0.so

clean:
	rm -f *.so *.o

libdumb.so: dumb.o ../libews0.so $(INC)
	$(GCC) -shared -Wl,-soname,libdumb.so -o libdumb.so dumb.o ../libews0.so -lc

dumb.o: dumb.c
	$(GCC) -c -fPIC -o dumb.o dumb.c

libhttp10.so: http10.o ../libews0.so $(INC)
	$(GCC) -shared -Wl,-soname,libhttp10.so -o libhttp10.so http10.o ../libews0.so -lc

http10.o: http10.c
	$(GCC) -c -fPIC -o http10.o http10.c

libmime.so: mime.o ../libews0.so $(INC)
	$(GCC) -shared -Wl,-soname,libmime.so -o libmime.so mime.o ../libews0.so -lc

mime.o: mime.c
	$(GCC) -c -fPIC -o mime.o mime.c
