.POSIX:
CC     = cc
CFLAGS = -W -O
PREFIX = /usr/local
LDLIBS = -lm
ALL_LDFLAGS = $(LDFLAGS) $(LIBS) -lxcb -lxcb-keysyms

all: xwm
install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp -f xwm $(DESTDIR)$(PREFIX)/bin
	cp -f xwm.1 $(DESTDIR)$(PREFIX)/share/man/man1
	chmod 755 $(DESTDIR)$(PREFIX)/bin/xwm
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man1/xwm.1
xwm: xwm.o
	$(CC) $(ALL_LDFLAGS) -o xwm xwm.o $(LDLIBS)
xwm.o: xwm.c xwm.h config.h
clean:
	rm -f xwm *.o
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/xwm
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/xwm.1
.PHONY: all install uninstall clean
