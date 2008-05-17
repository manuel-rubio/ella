all: ../libews0.so

clean:
	$(RM) -f ../libews0.so *.o

configurator.o: configurator.c $(INC)
	$(GCC) -c -o configurator.o configurator.c

connector.o: connector.c $(INC)
	$(GCC) -c -o connector.o connector.c

header.o: header.c $(INC)
	$(GCC) -c -o header.o header.c

logger.o: logger.c $(INC)
	$(GCC) -c -o logger.o logger.c

memory.o: memory.c $(INC)
	$(GCC) -c -o memory.o memory.c

string.o: string.c $(INC)
	$(GCC) -c -o string.o string.c

../libews0.so: configurator.o connector.o header.o memory.o string.o logger.o
	$(GCC) -shared -Wl,-soname,libews0.so -o ../libews0.so configurator.o connector.o header.o memory.o string.o logger.o -lc
