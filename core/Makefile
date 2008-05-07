all: ../ews

clean:
	rm -f ../ews *.o

configurator.o: configurator.c $(INC)
	$(GCC) -c -o configurator.o configurator.c

connector.o: connector.c $(INC)
	$(GCC) -c -o connector.o connector.c

modules.o: modules.c $(INC)
	$(GCC) -c -o modules.o modules.c

main.o: main.c $(INC)
	$(GCC) -c -o main.o main.c

../ews: configurator.o connector.o modules.o main.o ../libews0.so
	$(GCC) -ldl -pthread -o ../ews ../libews0.so configurator.o connector.o modules.o main.o