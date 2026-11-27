# hello: the smallest Zero app, and zero-kit's own test. Builds the way every App does.
CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter
ZK_MODULES = app display
include ../../zero-kit.mk

hello: hello.c $(ZK_SRC) $(ZK_HDR)
	$(CC) $(CFLAGS) $(ZK_CFLAGS) -o $@ hello.c $(ZK_SRC) $(LDLIBS) $(ZK_LDLIBS)

clean:
	rm -f hello

.PHONY: clean
