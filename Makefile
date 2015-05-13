CC=gcc
CFLAGS=-Wall -O3 -std=c99 `pkg-config --cflags MagickWand xcb`
LIBS=`pkg-config --libs MagickWand xcb`
BIN=xwobf

.PHONY: all clean

all: xwobf.o
	${CC} ${LIBS} xwobf.o -o ${BIN}

xwobf.o: xwobf.c
	${CC} ${CFLAGS} -c xwobf.c

clean:
	rm -rf *.o ${BIN}
