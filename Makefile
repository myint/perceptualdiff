default: build

NUM_JOBS := $(shell getconf _NPROCESSORS_ONLN)

build:
	mkdir -p build
	cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
	make --directory=build --jobs=$(NUM_JOBS)

clean:
	rm -rf build

install: build
	make --directory=build install

test: build
	make --directory=build check

.PHONY: build clean
