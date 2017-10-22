default: build

NUM_JOBS := $(shell getconf _NPROCESSORS_ONLN)

build:
	mkdir -p build
	test -f build/Makefile || cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
	$(MAKE) --directory=build --jobs=$(NUM_JOBS)

sanitizer:
	mkdir -p build
	test -f build/Makefile || cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DSANITIZERS=ON
	$(MAKE) --directory=build --jobs=$(NUM_JOBS) check

clean:
	rm -rf build

install: build
	$(MAKE) --directory=build install

test: build
	./test/run_tests.bash

.PHONY: build clean
