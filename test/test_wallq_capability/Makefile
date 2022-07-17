.phony: clean run

jsonLib := ../../src/ext/json.hpp
main: 
	g++ -o main main.cpp $(jsonLib) -std=c++20

clean:
	rm main

run:
	./main