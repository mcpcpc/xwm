.POSIX:
CC     = cc
CFLAGS = -W -O
PREFIX = /usr/local
LDLIBS = -lm
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man
LDFLAGS_REQUIRED = -lxcb -lxcb-keysyms

all: xwm
install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp -f xwm $(DESTDIR)$(BINDIR)
	cp -f xwm.1 $(DESTDIR)$(MANDIR)/man1
	chmod 755 $(DESTDIR)$(BINDIR)/xwm
	chmod 644 $(DESTDIR)$(MANDIR)/man1/xwm.1
xwm: xwm.o
	$(CC) $(LDFLAGS_REQUIRED) $(LDFLAGS) -o xwm xwm.o $(LDLIBS)
xwm.o: xwm.c xwm.h config.h
clean:
	rm -f xwm *.o
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/xwm
	rm -f $(DESTDIR)$(MANDIR)/man1/xwm.1
.PHONY: all install uninstall clean
