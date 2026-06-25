CC := gcc
CFLAGS := -Wall -Wextra -O2

FORMATS := png jpg gif bmp webp avif
BINS := $(addprefix build/,$(addsuffix dims,$(FORMATS)))

all: $(BINS)

build:
	mkdir -p build

build/%dims: %dims.c | build
	$(CC) $(CFLAGS) -o $@ $<

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(BINS) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(addprefix $(DESTDIR)$(PREFIX)/bin/,$(notdir $(BINS)))

clean:
	rm -rf build

.PHONY: all install uninstall clean
