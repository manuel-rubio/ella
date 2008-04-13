all: tornasauce

GCC=gcc -g

tornasauce: util/string.o util/header.o connector/connector.o config/config.o main/main.o modules/modules.o include/config.h
	$(GCC) -pthread -o tornasauce util/string.o connector/connector.o config/config.o main/main.o util/header.o modules/modules.o

util/string.o: include/util/string.h util/string.c include/config.h
	$(GCC) -c -o util/string.o util/string.c

util/header.o: include/util/header.h util/header.c include/config.h
	$(GCC) -c -o util/header.o util/header.c

connector/connector.o: include/util/string.h include/connector/connector.h connector/connector.c include/config.h
	$(GCC) -pthread -c -o connector/connector.o connector/connector.c

config/config.o: include/util/string.h include/config/config.h config/config.c include/config.h
	$(GCC) -c -o config/config.o config/config.c

main/main.o: main/main.c include/util/string.h include/connector/connector.h include/util/header.h
	$(GCC) -pthread -c -o main/main.o main/main.c

modules/modules.o: modules/modules.c include/modules/modules.h include/util/header.h include/config/config.h include/config.h
	$(GCC) -c -o modules/modules.o modules/modules.c

clean:
	rm -f tornasauce config/*.o connector/*.o config/*.o main/*.o util/*.o modules/*.o
