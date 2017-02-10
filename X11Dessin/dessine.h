// A COMPILER avce -lX11

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h> 



/**
 * initialisation fenetre
 * w: largeur
 * h:  hauteur
 * titre: titre fenetre
 * r,g,b couleur de fond
 */
void init_win(int w, int h, char* titre, float r, float g, float b);

/**
 * boucle evenementielle
 */
void event_loop();

/**
 * vide fenetre
 */
void clear_win();

/**
 * largeur fenetre
 */
int width_win();

/**
 * hauteur fenetre
 */
int height_win();

/**
 *  change couleur de trace en RVB
 *  r: %rouge [0,1]
 *  v: %vert [0,1]
 *  b: %bleu [0,1]
 */
void color(float r, float v, float b);

 
/**
 * trace une ligne
 * x0,y0 point depart
 * x1,y1 point arrivee
 */ 
void line(int x0, int y0, int x1, int y1);


/**
 * trace un rectangle
 * x,y position
 * w,h taille
 */
void rectangle(int x, int y, int w, int h);


/**
 * trace un rectangle plein
 * x,y position
 * w,h taille
 */
void filled_rectangle(int x, int y, int w, int h);

/**
 * trace un cercle
 * x,y position
 * r rayon
 */
void circle(int x, int y, int r);

/**
 * trace un disque
 * x,y position
 * r rayon
 */
void filled_circle(int x, int y, int r);

/**
 * trace un pixel a la position x,y
 */
void pixel(int x, int y);

/**
 * affiche une chaine de caractere
 */
void string(int x, int y, char* chaine);


