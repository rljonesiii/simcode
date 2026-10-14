# Convenience wrapper around the CMake presets in CMakePresets.json.
# (The original 1997-2002 makefiles are kept as makefile.mci,
# makefile.qnsim and makefile.bak for reference; they no longer build.)
#
#   make            configure, build and test with sanitizers (dev preset)
#   make release    optimized build and test
#   make lint       compile everything through clang-tidy
#   make golden     regenerate regression golden files (review the diff!)
#   make clean      delete all build trees

.PHONY: all test build release lint golden clean

all: test

test:
	cmake --workflow --preset dev

build:
	cmake --preset dev
	cmake --build --preset dev

release:
	cmake --workflow --preset release

lint:
	cmake --workflow --preset lint

golden: build
	cmake --build --preset dev --target golden

clean:
	rm -rf build
