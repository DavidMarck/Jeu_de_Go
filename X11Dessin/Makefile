all: exemple

clean:
	rm exemple dessine.o libdessine.a
	
exemple: exemple.c dessine.h libdessine.a
	gcc -g exemple.c -o exemple -L. -ldessine -lX11

libdessine.a: dessine.c dessine.h
	gcc -g -c dessine.c
	ar -r libdessine.a dessine.o
	
