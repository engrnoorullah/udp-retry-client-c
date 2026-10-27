CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test clean
all: build/udp_server build/udp_client
build:
	mkdir -p build
build/udp_server: src/server.c src/udp.h | build
	$(CC) $(CFLAGS) src/server.c -o $@
build/udp_client: src/client.c src/udp.h | build
	$(CC) $(CFLAGS) src/client.c -o $@
test: all
	python3 tests/test_network.py
clean:
	rm -rf build
