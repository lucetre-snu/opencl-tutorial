test: Test/test.cpp
	g++ Test/test.cpp -o Test/test -lOpenCL -I /usr/include -std=c++11
	./Test/test

hello: HelloWorld/hello.cpp
	g++ HelloWorld/hello.cpp -o HelloWorld/hello -lOpenCL -I /usr/include -std=c++11
	./HelloWorld/hello

ex1: Examples/ex1.c
	g++ Examples/ex1.c -o Examples/ex -lOpenCL 
	./Examples/ex

ex2: Examples/ex2.c
	g++ Examples/ex2.c -o Examples/ex -lOpenCL
	./Examples/ex

ex3: Examples/ex3.c
	g++ Examples/ex3.c -o Examples/ex -lOpenCL
	./Examples/ex

ex4: Examples/ex4.c
	g++ Examples/ex4.c -o Examples/ex -lOpenCL
	./Examples/ex
