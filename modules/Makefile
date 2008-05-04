all: $(MODULES) ../libtornasauce0.so

clean:
	rm -f *.so *.o

libdumb.so: dumb.o ../libtornasauce0.so $(INC)
	$(GCC) -shared -Wl,-soname,libdumb.so -o libdumb.so dumb.o ../libtornasauce0.so -lc

dumb.o: dumb.c
	$(GCC) -c -fPIC -o dumb.o dumb.c

libhttp10.so: http10.o ../libtornasauce0.so $(INC)
	$(GCC) -shared -Wl,-soname,libhttp10.so -o libhttp10.so http10.o ../libtornasauce0.so -lc

http10.o: http10.c
	$(GCC) -c -fPIC -o http10.o http10.c
