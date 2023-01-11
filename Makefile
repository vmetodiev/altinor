all: altinor

altinor:
	g++ -Wall -Werror -c -o signature_ocl_matrix.o signature_ocl_matrix.cpp
	g++ -Wall -Werror -c -o altinor.o altinor.cpp
	g++ -Wall -Werror -o $@ -L -o altinor.o signature_ocl_matrix.o -l OpenCL

clean:
	@rm -v signature_ocl_matrix.o
	@rm -v altinor.o
	@rm -v altinor