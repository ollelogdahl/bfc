SOURCES=$(shell find src/ -name '*.c')
HEADERS=$(shell find src/ -name '*.h')

OJBECTS=$(SOURCES:src/%.c=out/%.o)

CFLAGS = -g -Werror
LDFLAGS = -Isrc/

.PHONY: build clean test

out/bfc: $(OJBECTS) | out
	$(CC) -o out/bfc $^ $(LDFLAGS)

out/%.o: src/%.c $(HEADERS) | out
	$(CC) -c -o $@ $< $(CFLAGS)

out:
	mkdir -p out

build: out/bfc

# run test.sh
test: build
	./test.sh

clean:
	rm -rf out/