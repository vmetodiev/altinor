all: benchmark

benchmark:
	g++ -Wall -Werror -c -o dynmatrix.o dynmatrix.cpp
	g++ -Wall -Werror -c -o benchmark.o benchmark.cpp
	g++ -Wall -Werror -o $@ -L -o benchmark.o dynmatrix.o -l OpenCL

clean:
	@rm -v dynmatrix.o
	@rm -v benchmark.o
	@rm -v benchmark