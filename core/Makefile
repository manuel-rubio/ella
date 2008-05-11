all: ../ewsd

clean:
	rm -f ../ewsd *.o

configurator.o: configurator.c $(INC)
	$(GCC) -c -o configurator.o configurator.c

connector.o: connector.c $(INC)
	$(GCC) -c -o connector.o connector.c

console.o: console.c $(INC)
	$(GCC) -c -o console.o console.c

modules.o: modules.c $(INC)
	$(GCC) -c -o modules.o modules.c

main.o: main.c $(INC)
	$(GCC) -c -o main.o main.c

../ewsd: configurator.o connector.o modules.o main.o console.o ../libews0.so
	$(GCC) -ldl -pthread -o ../ewsd ../libews0.so configurator.o connector.o modules.o main.o console.o
