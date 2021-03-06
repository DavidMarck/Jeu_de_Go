#include "dessine.h"

#define true 1
#define false 0
#define bool int

/** 
 * énumération des couleurs de pierre
 * BLANC : pierre blanche
 * NOIR : pierre Noire
 */
typedef enum eCouleurPierre
{
    BLANC = 1,
    NOIR = 2
} CouleurPierre;

/**
 *  énumération des différents types d'intersections (en fonction de leur position)
 *  COIN_HG, COIN_HD, COIN_BG, COIN_BD : respectivement les quatres coins du plateau 
 *	BORD_HAUT, BORD_DROIT, BORD_BAS, BORD_GAUCHE : les quatres bords du plateau
 *	DEFAUT : les autres intersections
 */ 
typedef enum eTypeInter 
{
	COIN_HG,
	COIN_HD,
	COIN_BG,
	COIN_BD,
	BORD_HAUT,
	BORD_DROIT,
	BORD_BAS,
	BORD_GAUCHE,
	DEFAUT
} TypeInter;

/**
 * énumération qui indique le mode de jeu
 * MENU : mode de choix du plateau
 * JEU : mode de jeu
 */
typedef enum eMode
{
	MENU, 
	JEU
} Mode;

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
 * nbLibertes : nombre de libertes de l'intersection
 * estOccupe : si une pierre est posée sur l'intersection ou non
 * couleur : couleur de la pierre si l'intersection est occupée
 * type : Correspond au type d'emplacement de l'intersection (ex : coin haut)
 * suiteChaine : Intersection suivante dans la chaine
 * chMere : chaine à laquelle appartient l'intersection
 */ 
typedef struct sIntersection
{
	Coord* position;	
	int nbLibertes;
	bool estOccupe;
	CouleurPierre couleur;
	TypeInter type;
	struct sIntersection* suiteChaine;
	struct sChaine* chMere;
} Intersection;

/**
 * contient toutes les intersections d'une chaîne
 * nbPierres : le nombre de pierres dans la chaine
 * debutChaine : La première intersection de la chaine
 * finChaine : La dernière intersection de la chaine
 */
typedef struct sChaine
{
	int nbPierres;
	Intersection* debutChaine;
	Intersection* finChaine;
} Chaine;


/**
 * Met en place l'ensemble des éléments graphiques du jeu
 */
void draw_win_plateau();


/**
 * Dessine la fenêtre relative au menu
 */
void draw_win_menu();

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
 * Dessine les pierres selon les données contenu dans le tableau lesInters
 */
 void dessine_Pierres();
 
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
Intersection* getIntersection(int x, int y);

/**
 * retourne, si elle existe, l'intersection située en haut de celle en entrée
 * inter : L'intersection dont on souhaite obtenir celle située au-dessus
 */
Intersection* getIntersectionHaut(Intersection* inter);

/**
 * retourne, si elle existe, l'intersection située à droite de celle en entrée
 * inter : L'intersection dont on souhaite obtenir celle située à droite
 */
Intersection* getIntersectionDroite(Intersection* inter);

/**
 * retourne, si elle existe, l'intersection située en bas de celle en entrée
 * inter : L'intersection dont on souhaite obtenir celle située au-dessous
 */
Intersection* getIntersectionBas(Intersection* inter);

/**
 * retourne, si elle existe, l'intersection située à gauche de celle en entrée
 * inter : L'intersection dont on souhaite obtenir celle située à gauche
 */
Intersection* getIntersectionGauche(Intersection* inter);

/**
 * Rattache une pierre à une chaine
 * chaine : la chaine ciblée par l'ajout
 * pierre : la pierre à ajouter
 */
void incrementChaine(Chaine* chaine, Intersection* pierre);

/**
 * Initialise une chaine avec une intersection
 * inter : la première intersection de la chaine
 */
void initChaine(Intersection* inter);

/**
 * Indique si il est permis de jouer sur cette intersection ou non
 */
bool coupEstPermis(Intersection* inter);

/**
 * actualise le nombre de libertés des intersections du plateau
 */
void updateNbLibertes();

/**
 * Change la valeur du tour
 */
void changerTour();

/**
 * Retourne le rayon d'une pierre en fonction des dimensions du plateau actuel
 */
float getRayonPierre();

/**
 * Vérifie s'il s'agit d'un coin ou non
 * inter : intersection à tester
 */
bool estCoin(Intersection* inter);

/**
 * Vérifie s'il s'agit d'une intersection en bordure du goban (hors coins)
 * inter : intersection à tester
 */
bool estBordure(Intersection* inter);

/**
 * Attribue le nombre de libertés à la création des intersections
 * i,j : respectivement ligne et colonne
 * inter : intersection dont l'attribut nbLibertes va être modifié
 */
void setNbLibertes(Intersection* inter);

/**
 * Attribue le type d'intersection lors de la création de chacunes d'elles 
 * inter : l'intersection dont on veut les libertés
 */ 
void setTypeIntersection (Intersection* inter);

/**
 * Cette fonction crée ou fusionne les chaînes lors de la pose d'une pierre
 * inter : l'intersection sur laquelle on a posé une pierre
 */
void chainage(Intersection* inter);

/**
 *	Met à jour les libertés des différentes chaines et supprime les chaines capturées
 */
void updateLibertesChaines();

/**
 *	Met à jour le plateau dans son intégralité, il appelle updateLibertesChaines et redessine les éléments
 * 	(pour supprimer les chaines qui se font capturer)
 */
void updatePlateau ();


/**
 * Vérifie à la pose d'une pierre si une nouvelle chaîne va être créée ou non
 * inter : l'intersection sur laquelle on pose une pierre
 */
 bool estNouvChaine(Intersection* inter);
 
 /**
  * Retourne un tableau contenant les intersections adjacentes
  * inter : intersection dont on retounre les adjacentes
  * taille : taille du tabelau retourné (pour l'allocation)
  */
 Intersection** getLesAdjacents(Intersection* inter, int taille);
 
 /**
  * Retourne le nombre d'intersections adjacentes
  * inter : intersection dont on veut connaître le nombre d'adjacences
  */
 int getNbAdjacents(Intersection* inter);
 
 /**
  * retourne un tableau contenant les libertes d'une intersection
  * inter : intersection dont on veut obtenir les libertes
  */
Intersection** getLesLibertes(Intersection* inter);

/**
 * Détermine si au moins une intersection adjacente est occupée par une pierre
 * inter : intersection à verfier
 */
 bool aAdjacentOccupe(Intersection* inter);

/**
 * Retourne le nombre total des libertés d'une chaîne
 * chaine : la chaîne dont on souhaite connaître le nombre de libertés
 */
int getNbLibertesChaine(Chaine* chaine);

/**
 * Retourne le nombre de liberté total que peut avoir une intersection
 * inter : l'intersection en question
 */
int getNbLibertesTotal(Intersection* inter);

/**
 * Supprime une chaîne de lesChaînes
 */
 void supprimeChaine(Chaine* chaine);

/**
 * Libère la globalité des ressources mémoires utilisées
 */
void freeAll();


