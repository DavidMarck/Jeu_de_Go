all: jeu

clean:
	rm jeu dessine.o libdessine.a
	
jeu: jeu.c dessine.h libdessine.a
	gcc -Wall -g jeu.c -o jeu -L. -ldessine -lX11

libdessine.a: dessine.c dessine.h
	gcc -Wall -g -c dessine.c
	ar -r libdessine.a dessine.o
	
