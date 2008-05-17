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
       ../include/logger.h \
       ../include/memory.h \
       ../include/string.h

all: ewsd

ewsd: common core modules core/ewsd.c
	$(MAKE) -C common
	$(MAKE) -C core
	$(MAKE) -C modules

ews: common core modules core/ews.c
	$(MAKE) -C common
	$(MAKE) -C core
	$(MAKE) -C modules

clean:
	$(MAKE) -C common clean
	$(MAKE) -C core clean
	$(MAKE) -C modules clean
	rm -f ews.ctl
