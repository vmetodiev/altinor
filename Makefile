all: altinor

altinor:
	g++ -Wall -Werror -c -o dynmatrix.o dynmatrix.cpp
	g++ -Wall -Werror -c -o altinor.o altinor.cpp
	g++ -Wall -Werror -o $@ -L -o altinor.o dynmatrix.o -l OpenCL

clean:
	@rm -v dynmatrix.o
	@rm -v altinor.o
	@rm -v altinor