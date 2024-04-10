libhello.a:mymath.o myprint.o
	ar -rc libhello.a mymath.o myprint.o
mymath.o:mymath.c
	gcc -c mymath.c -o mymath.o
myprint.o:myprint.c
	gcc -c myprint.c -o myprint.o
.PHONY:hello 
hello:
	mkdir -p hello/lib  
	mkdir -p hello/include 
	cp *.a hello/lib 
	cp *.h hello/include 
.PHONY:clean
clean:
	rm -rf libhello.a mymath.o myprint.o hello
