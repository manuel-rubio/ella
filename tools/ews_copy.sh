#!/bin/bash

if [ -z "$1" ]; then
	echo "Uso: $(basename $0) VERSION"
	exit
fi

DEST=../tags/ews-$1

cd $(dirname $0)/..

DATA="common include main modules share arch/*"

cp -vf AUTHORS COPYING INSTALL Makefile.in README dist.sh configure.ac $DEST
for i in $DATA; do
	cp -vf $i/* $DEST/$i
done
cp -vf etc/ews/http.ini $DEST/etc/ews/

