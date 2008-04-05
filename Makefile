all: tornasauce

GCC=gcc -g

tornasauce: util/string.o util/header.o connector/connector.o config/config.o main/main.o config.h
	$(GCC) -o tornasauce util/string.o connector/connector.o config/config.o main/main.o util/header.o

util/string.o: util/string.h util/string.c config.h
	$(GCC) -c -o util/string.o util/string.c

util/header.o: util/header.h util/header.c config.h
	$(GCC) -c -o util/header.o util/header.c

connector/connector.o: util/string.h connector/connector.h connector/connector.c config.h
	$(GCC) -c -o connector/connector.o connector/connector.c

config/config.o: util/string.h config/config.h config/config.c config.h
	$(GCC) -c -o config/config.o config/config.c

main/main.o: main/main.c util/string.h connector/connector.h util/header.h
	$(GCC) -c -o main/main.o main/main.c

clean:
	rm -f tornasauce config/*.o connector/*.o config/*.o main/*.o util/*.o
