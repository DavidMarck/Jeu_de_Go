#include "dessine.h"

#define true 1
#define false 0
#define bool int

/** 
 * énumération des couleurs de pierre
 */
typedef enum ePierre
{
    BLANC, NOIR
} Pierre;

//~ typedef struct sRectangle 
//~ {
	//~ float x0;
	//~ float x1;
	//~ float y0;
	//~ float y1;
//~ } Rectangle;

/** 
 * repésente une coordonnée avec position en x et posotion en y
 * x,y : les positions x et y
 */
typedef struct sCoord
{
	int posX;
	int posY;
} Coord;

/**
 * représente une intersection du goban (plateau)
 * position : Coord (x,y)
 * estOccupe : si une pierre est posée sur l'intersection ou non
 * couleur : couleur de la pierre si l'intersection est occupée
 */ 
typedef struct sIntersection
{
	Coord* position;
	struct sIntersection* fh;
	struct sIntersection* fd;
	struct sIntersection* fb;
	struct sIntersection* fg;	
	struct sIntersection* lesLibertes;
	bool estOccupe;
	Pierre couleur;
	
} Intersection;

/**
 * représente les liaisons d'une intersection racine avec les intersections adjacentes sur le goban
 * (la relation entre ces Chaine permet d'établir les chaînes du plateau)
 * racine : la pierre pointée actuellement
 * fh : l'intersection située en haut (s'il n'y a pas de pierre, la valeur est "null")
 * fd : l'intersection située à droite (s'il n'y a pas de pierre, la valeur est "null")
 * fb : l'intersection située en bas (s'il n'y a pas de pierre, la valeur est "null")
 * fg : l'intersection située à gauche (s'il n'y a pas de pierre, la valeur est "null")
 * lesLibertes :
 */
typedef struct sChaine
{
	Intersection* racine;
	
} Chaine;


/**
 * Met en place l'ensemble des éléments graphiques du jeu
 */
void draw_win_plateau();

/**
 * Création du plateau de jeu
 * (appelée dans draw_win_plateau)
 */
void dessine_plateau();

/**
 * Trace les hoshis de droite à gauche en fonction de la dimension du plateau
 * (appelée dans draw_win_plateau)
 */ 
void dessine_hoshi();

/**
 * Remplissage du tableau contenant l'ensemble des intersections
 */
Intersection** creerTableInter();

/**
 * Retourne la dimension des cases selon la dimension du plateau
 */
float getCoteCase();

/**
 * Crée et retourne une nouvelle instance de Coordonnee
 * x,y : coordonnées x et y
 */
Coord* nouvCoord(int x, int y);

/**
 * Crée et retourne une instance de Intersection "vide" (i.e. positions x et y à 0)
 */
Intersection* initInterVide();

/**
 * Teste si une intersection est "vide" (x = 0 et y = 0)
 * inter : l'intersection qu'on vérifie
 */
bool interEstVide(Intersection* inter);

/**
 * Retourne l'intersection sur laquelle le joueur a cliqué
 * x,y : coordonnées x et y (du clic)
 */
Intersection* getPlacement(int x, int y);

/**
 * Change la valeur "estOccupe" d'une Intersection
 * coord : coordonnées x, y
 * estOccupe : true ou false
 */
void setEstOccupe(Coord* coord, bool estOccupe);

/**
 * Change la valeur du tour
 */
void changerTour();

/**
 * Retourne le rayon d'une pierre en fonction des dimensions du plateau actuel
 */
float getRayonPierre();

/**
 * Libère la globalité des ressources mémoires utilisées
 */
void freeAll();


