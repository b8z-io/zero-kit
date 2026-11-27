# zero-kit itself is a set of sources Apps compile in (see zero-kit.mk). This Makefile builds and checks the
# example App the way CI does.
all:
	$(MAKE) -C examples/hello

test: all
	tests/run.sh

# After an intended change to what hello draws: rewrite the stored expected frames, then look at them.
update-expected: all
	UPDATE=1 tests/run.sh

clean:
	$(MAKE) -C examples/hello clean
	rm -rf test

.PHONY: all test update-expected clean
