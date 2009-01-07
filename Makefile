irssi_inc=/opt/local/include/irssi

CC=gcc
CFLAGS=-I/opt/local/include -I${irssi_inc} -I${irssi_inc}/src
CFLAGS+=-I${irssi_inc}/src/core -I${irssi_inc}/src/fe-common/core
CFLAGS+=`glib-config --cflags glib`
CFLAGS+=-IGrowl.framework/Headers
CFLAGS+=-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc
LDFLAGS=-L/opt/local/lib `glib-config --libs glib`
LDFLAGS+=-arch i386 -arch ppc
LDFLAGS+=-bundle -flat_namespace -undefined suppress -framework Growl -F./

TARGETS=growl-irssi.o

growl: ${TARGETS}
	${CC} ${CFLAGS} -o libgrowl.dylib ${TARGETS} ${LDFLAGS}

clean:
	rm -f ${TARGETS} libgrowl.dylib
