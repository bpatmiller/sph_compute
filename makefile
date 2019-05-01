default: build

.PHONY: build
build:
	-[[ -d build ]] || mkdir build
	cd build; cmake ..;  make -j8

.PHONY: format
format:
	clang-format -i src/*.cpp src/*.h src/shaders/*.vert src/shaders/*.frag

.PHONY: clean
clean:
	rm -rf build/

.PHONY: run
run:
	build/bin/simulation

