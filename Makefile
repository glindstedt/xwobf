CC ?= gcc

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

CFLAGS = -Wall -std=c99 `pkg-config --cflags MagickWand xcb`

LIBS   = `pkg-config --libs MagickWand xcb`

BIN = xwobf

.PHONY: all debug clean install uninstall

all: CFLAGS += -O3
all: $(BIN)

debug: CFLAGS += -DDEBUG -g
debug: $(BIN)

install: $(BIN)
	@install -d "$(DESTDIR)$(BINDIR)"
	@install -m755 $(BIN) "$(DESTDIR)$(BINDIR)"

uninstall:
	@rm -f "$(DESTDIR)$(BINDIR)/xwobf"

xwobf: xwobf.o
	$(CC) $(LIBS) xwobf.o -o $(BIN)

xwobf.o: xwobf.c
	$(CC) $(CFLAGS) -c xwobf.c

clean:
	rm -rf *.o $(BIN)
