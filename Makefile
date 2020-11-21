.POSIX:
CC = c99
ALL_WARNING = -Wall -Wextra
ALL_LDFLAGS = -lxcb -lxcb-keysyms $(LDFLAGS)
ALL_CFLAGS = $(ALL_WARNING) $(CPPFLAGS) $(CFLAGS)
PREFIX = /usr/local
LDLIBS = -lm
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

all: xwm
install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp -f xwm $(DESTDIR)$(BINDIR)
	cp -f xwm.1 $(DESTDIR)$(MANDIR)/man1
	chmod 755 $(DESTDIR)$(BINDIR)/xwm
	chmod 644 $(DESTDIR)$(MANDIR)/man1/xwm.1
xwm: xwm.o
	$(CC) $(ALL_LDFLAGS) -o xwm xwm.o $(LDLIBS)
xwm.o: xwm.c xwm.h config.h
	$(CC) $(ALL_CFLAGS) -c xwm.c
clean:
	rm -f xwm *.o
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/xwm
	rm -f $(DESTDIR)$(MANDIR)/man1/xwm.1
.PHONY: all install uninstall clean
