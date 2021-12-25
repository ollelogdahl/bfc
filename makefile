SOURCES=$(shell find src/ -name '*.c')
HEADERS=$(shell find src/ -name '*.h')

OJBECTS=$(SOURCES:src/%.c=out/%.o)

CFLAGS = -std=c99 -Werror
LDFLAGS = -Isrc/

.PHONY: build clean test install

out/bfc: $(OJBECTS) | out
	$(CC) -o out/bfc $^ $(LDFLAGS)

out/%.o: src/%.c $(HEADERS) | out
	$(CC) -c -o $@ $< $(CFLAGS)

out:
	mkdir -p out

build: out/bfc
	chmod +x out/bfc

# run test.sh
test: build
	./check.sh out/bfc nocolor

clean:
	rm -rf out/

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: build
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m +x out/bfc $(DESTDIR)$(PREFIX)/bin/
