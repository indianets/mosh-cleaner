CFLAGS ?= -O3 -march=native -pipe -fomit-frame-pointer -fPIE -fstack-protector-all
LDFLAGS ?= -Wl,-z,now -Wl,-z,relro
PREFIX ?= /usr/local

.PHONY: clean install

all: clean-mosh

clean:
	rm -f clean-mosh

install:
	install -m 0100 -o 0 -g 0 -t $(PREFIX)/sbin/ clean-mosh

clean-mosh:
