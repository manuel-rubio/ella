all: ../ewsd ../ews

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

ewsd.o: ewsd.c $(INC)
	$(GCC) -c -o ewsd.o ewsd.c

ews.o: ews.c $(INC)
	$(GCC) -c -o ews.o ews.c

../ews: ews.o
	$(GCC) -pthread -o ../ews ews.o

../ewsd: configurator.o connector.o modules.o ewsd.o console.o ../libews0.so
	$(GCC) -ldl -pthread -o ../ewsd ../libews0.so configurator.o connector.o modules.o ewsd.o console.o
