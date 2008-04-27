all: tornasauce

GCC=gcc -g

all: tornasauce modules/libdumb.so modules/libhttp10.so

tornasauce: core/string.o core/header.o core/connector.o core/configurator.o core/main.o core/modules.o include/config.h
	$(GCC) -ldl -pthread -o tornasauce core/string.o core/connector.o core/configurator.o core/main.o core/header.o core/modules.o

core/string.o: include/string.h core/string.c include/config.h
	$(GCC) -c -o core/string.o core/string.c

core/header.o: include/header.h core/header.c include/config.h
	$(GCC) -c -o core/header.o core/header.c

core/connector.o: include/string.h include/connector.h core/connector.c include/config.h
	$(GCC) -c -o core/connector.o core/connector.c

core/configurator.o: include/string.h include/configurator.h core/configurator.c include/config.h
	$(GCC) -c -o core/configurator.o core/configurator.c

core/main.o: core/main.c include/string.h include/connector.h include/header.h
	$(GCC) -c -o core/main.o core/main.c

core/modules.o: core/modules.c include/modules.h include/header.h include/configurator.h include/config.h
	$(GCC) -c -o core/modules.o core/modules.c

clean:
	rm -f tornasauce core/*.o modules/*.o modules/*.so



modules/libdumb.so: modules/dumb.o core/string.o core/header.o include/string.h include/header.h
	$(GCC) -shared -Wl,-soname,libdumb.so -o modules/libdumb.so modules/dumb.o core/string.o core/header.o -lc

modules/dumb.o: modules/dumb.c
	$(GCC) -c -fPIC -o modules/dumb.o modules/dumb.c

modules/libhttp10.so: modules/http10.o core/string.o core/header.o core/connector.o core/configurator.o include/string.h include/header.h
	$(GCC) -shared -Wl,-soname,libhttp10.so -o modules/libhttp10.so modules/http10.o core/configurator.o core/connector.o core/string.o core/header.o -lc

modules/http10.o: modules/http10.c
	$(GCC) -c -fPIC -o modules/http10.o modules/http10.c
