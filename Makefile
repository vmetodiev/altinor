all: experiment

experiment:
	g++ -Wall -Werror -c -o dynmatrix.o dynmatrix.cpp
	g++ -Wall -Werror -c -o experiment.o experiment.cpp
	g++ -Wall -Werror -o $@ -L -o experiment.o dynmatrix.o -l OpenCL

clean:
	@rm -v dynmatrix.o
	@rm -v experiment.o
	@rm -v experiment