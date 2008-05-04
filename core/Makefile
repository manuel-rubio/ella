all: ../tornasauce

clean:
	rm -f ../tornasauce *.o

configurator.o: configurator.c $(INC)
	$(GCC) -c -o configurator.o configurator.c

connector.o: connector.c $(INC)
	$(GCC) -c -o connector.o connector.c

modules.o: modules.c $(INC)
	$(GCC) -c -o modules.o modules.c

main.o: main.c $(INC)
	$(GCC) -c -o main.o main.c

../tornasauce: configurator.o connector.o modules.o main.o ../libtornasauce0.so
	$(GCC) -ldl -pthread -o ../tornasauce ../libtornasauce0.so configurator.o connector.o modules.o main.o