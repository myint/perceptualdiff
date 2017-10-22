default: build

NUM_JOBS := $(shell getconf _NPROCESSORS_ONLN)

build:
	mkdir -p build
	cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
	$(MAKE) --directory=build --jobs=$(NUM_JOBS)

clean:
	rm -rf build

install: build
	$(MAKE) --directory=build install

test: build
	$(MAKE) --directory=build check

.PHONY: build clean
