#!/bin/sh

cd .. 
tar cz --exclude=.svn -f ews-0.1-`date +%Y%m%d`.tar.gz ella-src/{etc,main,common,modules,AUTHORS,ChangeLog,COPYING,include,INSTALL,configure,Makefile.in,README}

