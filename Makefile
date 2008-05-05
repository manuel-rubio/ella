export GCC
export MODULES
export INC

GCC = gcc -g

MODULES = libdumb.so \
          libhttp10.so \
	  libmime.so

INC := ../include/configurator.h \
       ../include/config.h \
       ../include/connector.h \
       ../include/header.h \
       ../include/memory.h \
       ../include/string.h

all: tornasauce

tornasauce: common core modules
	$(MAKE) -C common
	$(MAKE) -C core
	$(MAKE) -C modules

clean:
	$(MAKE) -C common clean
	$(MAKE) -C core clean
	$(MAKE) -C modules clean
