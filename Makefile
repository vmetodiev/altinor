all: experimentNG

experimentNG:
	g++ -Wall -Werror -c -o dynmatrix.o dynmatrix.cpp
	g++ -Wall -Werror -c -o experimentNG.o experimentNG.cpp
	g++ -Wall -Werror -o $@ -L -o experimentNG.o dynmatrix.o -l OpenCL

clean:
	@rm -v dynmatrix.o
	@rm -v experimentNG.o
	@rm -v experimentNG