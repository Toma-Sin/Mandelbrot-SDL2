# Default target, compiles and runs the program
.PHONY: default
default: clean compile run

.PHONY: all
all: clean compile run docs

# Compiles the program
.PHONY: build
build: main.cpp
	g++ -Wall -pedantic main.cpp -o mandel

# Runs the program
.PHONY: run
run: mandel
	./mandel

# Generates documentation
.PHONY: docs
docs:
	doxygen Doxyfile

# Clean up old build artifacts
.PHONY: clean
clean:
	rm -f mandel

