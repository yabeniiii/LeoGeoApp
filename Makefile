.PHONY: setup build clean build-release

setup:
	 conan install . -sbuild_type=Debug --output-folder=build --build=missing && cd build && cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake ..

build:
	cd build && cmake --build .

clean:
	rm -rf build

build-release:
	conan install . -sbuild_type=Release --output-folder=build --build=missing && cd build && cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
