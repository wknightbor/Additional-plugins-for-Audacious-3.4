#!/bin/sh
#gcc -fPIC -I/usr/include/glib-2.0 -c cue.c `pkg-config --cflags --libs glib-2.0`
aclocal
autoheader
automake --add-missing
autoconf
./configure --prefix=/usr
make
