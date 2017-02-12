#include "jeu.h"


const float COTE_CASE_19 = 30; // côté des cases plateau 19x19 lignes (18x18 cases)
const float RAYON_PIERRE_19 = 10; // rayon d'une pierre sur un plateau 19x19

const float COTE_CASE_13 = 45; // côté des cases plateau 13x13 lignes (12x12 cases) ==> 18/12 * COTE_CASE_19
const float RAYON_PIERRE_13 = 15; // rayon d'une pierre sur un plateau 13x13

const float COTE_CASE_9 = 67.5; // côté des cases plateau 9x9 lignes (8x8 cases) ==> 18/8 * COTE_CASE_19
const float RAYON_PIERRE_9 = 22.5; // rayon d'une pierre sur un plateau 9x9

const int MARGE_FEN = 25;

static int dims_plateau; // dimensions du plateau choisies (19, 13 ou 9)
static Intersection** lesInters; // tableau de structures contenant les intersections du goban (plateau)
static Chaine** lesChaines;	// tableau de structures contenant les chaines du goban au fil de la partie(plateau)
static int nbChaines = 0;
static CouleurPierre tour = NOIR;  // indique le tour actuel

/**
 * Mettre ici son code pour dessiner dans la fenetre (plateau)
 * 1er affichage + redessine si resize
 */
void draw_win_plateau()
{	
	// vide la fenetre
	clear_win();
	// on récupère la longueur du côté des cases en fonction de la dimension du plateau
	float coteCase = getCoteCase();
	color(1, 0, 0);
	rectangle(MARGE_FEN,MARGE_FEN,(dims_plateau-1)*coteCase, (dims_plateau-1)*coteCase);
	
	dessine_plateau();
	dessine_hoshi();
	
	// Couleur de la première pierre qui sera posée (noir)
	color(0, 0, 0);
}

/**
 * Mettre ici son code pour dessiner dans la fenetre (niveau)
 * 1er affichage + redessine si resize
 */
void draw_win_niveau()
{	
	clear_win();
	
	//~ Rectangle* btn19 = malloc(sizeof(Rectangle));
	//~ btn19->x0 = MARGE_FEN;
	//~ btn19->y0 = MARGE_FEN ;
	//~ btn19->x1 = btn19->x0 + width_win() - 2*MARGE_FEN;
	//~ btn19->y1 = btn19->y0 + 90;
	
	rectangle(MARGE_FEN,MARGE_FEN,width_win() - 2*MARGE_FEN,90);
	string(width_win()/2,(MARGE_FEN + 45),"19x19");
	
	rectangle(MARGE_FEN,2*MARGE_FEN + 90,width_win() - 2*MARGE_FEN,90);
	string(width_win()/2,(MARGE_FEN + 45)+(MARGE_FEN+90),"13x13");
	
	rectangle(MARGE_FEN,3*MARGE_FEN + 2*90,width_win() - 2*MARGE_FEN,90);
	string(width_win()/2,(MARGE_FEN + 45)+2*(MARGE_FEN+90),"9x9");
}


/**
 * on a cliqué a la souris (plateau):
 * bouton: 1,2,3,4,5,... : gauche, milieu, droit, molette, ...
 * x,y position
 * dim dimension du plateau en cours
 */
void mouse_clicked_plateau(int bouton, int x, int y)
{
	printf("Bouton %d presse au coord. %d,%d \n",bouton,x,y);
	
	Intersection* inter = getIntersection(x, y); // on récupère l'intersection sur laquelle on va placer la pierre (x,y du clic)
	//~ printf("Haut = %p\tDroite = %p\tBas = %p\tGauche = %p\n", getIntersectionHaut(inter), getIntersectionDroite(inter), getIntersectionBas(inter), getIntersectionGauche(inter));
	
	if(coupEstPermis(inter)) // Si le coup est permis
	{
		filled_circle(inter->position->posX,inter->position->posY,getRayonPierre()); // ... on la remplit...
		inter->estOccupe = true; // ... on définit que l'intersection est maintement occupée...
		inter->couleur = tour;
		//~ printf("même couleur ? : %d\n", estMemeCouleur(inter, getIntersectionGauche(inter)));
		checkLesAdjacents(inter);
		printChaines();
		changerTour(); // ... et enfin on passe au tour suivant
	} 
	
}

//~ void updateLibertes(Intersection* interPierrePosee) 
//~ {
	//~ if (estCoin(int
	//~ switch (interPierrePosee->type)
	//~ {
		//~ case CO
	//~ }
//~ }

void changerTour()
{
	switch(tour)
	{
		// la dernière pierre posée était noire, on change la couleur pour la prochaine pierre
		case NOIR:
			color(1,1,1);
			tour = BLANC;
			break;
		// idem pour pour le blanc
		case BLANC:
			color(0,0,0);
			tour = NOIR;
			break;
		default:
			break;
	}
}

Intersection* getIntersection(int x, int y) 
{
	Intersection* inter = initInterVide(); // on initialise une Intersection quelconque ("vide")
	
	int posX = 0; // position X (voir boucle)
	int posY = 0; // position Y (voir boucle)
	
	int hitbox_placement = getRayonPierre(); // définit la zone cliquable autour d'une intersection pour le placement d'une pierre (= rayon d'une pierre)
	
	
	// on parcourt l'ensemble des intersections du plateau...
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			posX = lesInters[i * dims_plateau + j]->position->posX; // récupère la coordonnée X de l'intersection
			posY = lesInters[i * dims_plateau + j]->position->posY; // récupère la coordonnée Y de l'intersection

			// si l'utilisateur a cliqué dans le rayon de l'intersection...
			if((posX - hitbox_placement < x) && (x < posX + hitbox_placement) && (posY -hitbox_placement < y) && (y < posY + hitbox_placement)) 
			{
				inter = lesInters[i * dims_plateau + j]; // ... on retourne l'intersection
				break;
			}
		}
	}
	return inter;
}

bool coupEstPermis(Intersection* inter)
{
	bool estVide = interEstVide(inter); // si l'intersection est vide (x=0;y=0), alors c'est que l'utilisateur a cliqué en dehors des intersections cliquables
	bool estOccupe = inter->estOccupe; // on vérifie que l'intersection concernée ne soit pas occupée
	int nbLibertes = inter->nbLibertes;
	
	if(estVide == false && estOccupe == false && nbLibertes > 0) // si l'intersection n'est pas vide, ni occupée, qu'elle a des libertés et qu'il ne s'agit pas d'une capture...
	{
		return true;
	}
	return false;
}


/**
 * on a cliqué a la souris (choix du niveau):
 * bouton: 1,2,3,4,5,... : gauche, milieu, droit, molette, ...
 * x,y position
 */
void mouse_clicked_niveau(int bouton, int x, int y)
{
	printf("Bouton %d presse au coord. %d,%d \n",bouton,x,y);
	color( 1.0,0.0,1.0);
	filled_circle(x,y,10);
	
}

/**
 * on a appuyé sur une touche
 * code: code touche x11 (XK_...)
 * c caractère correspondant si caractere
 * x_souris,y_souris position de la souris
 */
void key_pressed(KeySym code, char c, int x_souris, int y_souris)
{
	switch(code)
	{
		case XK_Down:
			printf("bas\n");
			break;
		case XK_Up:
			printf("haut\n");
			break;
		case XK_Left:
			printf("gauche\n");
			break;
		case XK_Right:
			printf("droite\n");
			break;
	//~ case XK_Return:
	//~ case XK_Shift_L:
	//~ case XK_Control_R:
	//~ ....
		default:
			break;
	}
	
	if (c>' ' && c<'z')
		printf("char: %c \n",c);

	printf(" avec pos souris: %d,%d \n",x_souris,y_souris);

}

void dessine_plateau()
{
	float cote_Case = getCoteCase(); // on récupère la longueur d'un côté d'une case
	
	// on parcourt la fenêtre de gauche à droite et on dessine les lignes
	for(float i = MARGE_FEN + cote_Case; i < width_win() - MARGE_FEN; i += cote_Case)
	{
		line(i,MARGE_FEN, i, height_win()-MARGE_FEN);
	}
	// Même chose de haut en bas
	for(float i = MARGE_FEN + cote_Case; i < height_win() - MARGE_FEN; i += cote_Case)
	{
		line(MARGE_FEN,i, width_win()-MARGE_FEN, i);
	}
}

void dessine_hoshi()
{
	int posDep; // définit la position du premier hoshi
	int posFin; // définit la position du dernier hoshi
	int saut; // définit l'espace entre les hoshis
	float cote_Case = getCoteCase(); // longueur du côté d'une case
	
	// selon la dimension du plateau ...
	switch(dims_plateau)
	{
		case 19:
			posDep = 3;
			posFin = 16;
			saut = 6;
			break;
		
		case 13:
			posDep = 3;
			posFin = 10;
			saut = 6;
			filled_circle(MARGE_FEN+(6*cote_Case), MARGE_FEN + (6*cote_Case), 5); // on dessine le hoshi au centre du plateau (cas plateau 13x13 uniquement)
			break;
			
		case 9:
			posDep = 2;
			posFin = 7;
			saut = 4;
			break;
	}
	
	// on parcourt les différents emplacements de hoshi et on les dessine
	for(int x = posDep; x < posFin; x += saut) 
	{
		for(int y = posDep; y < posFin; y += saut)
		{
			filled_circle(MARGE_FEN+(x*cote_Case), MARGE_FEN + (y*cote_Case), 5);
		}
	}
}

float getCoteCase()
{
	if(dims_plateau == 19)
	{
		return COTE_CASE_19;
	}
	else if(dims_plateau == 13)
	{
		return COTE_CASE_13;
	}
	else if(dims_plateau == 9)
	{
		return COTE_CASE_9;
	}
	
	return 0;
}

float getRayonPierre()
{
	if(dims_plateau == 19)
	{
		return RAYON_PIERRE_19;
	}
	else if(dims_plateau == 13)
	{
		return RAYON_PIERRE_13;
	}
	else if(dims_plateau == 9)
	{
		return RAYON_PIERRE_9;
	}
	
	return 0;
}

Coord* nouvCoord(int x, int y)
{
	Coord* coord = malloc(sizeof(Coord));
	coord->posX = x;
	coord->posY = y;
	return coord;
}

Intersection*  nouvIntersection(Coord* coord)
{
	Intersection* inter = malloc(sizeof(Intersection));
	
	inter->position = coord;
	inter->estOccupe = false;
	
	return inter;
}

Intersection* initInterVide()
{
	Intersection* interVide = malloc(sizeof(Intersection));
	
	interVide->position = nouvCoord(0,0);
	
	return interVide;
}

bool interEstVide(Intersection* inter)
{
	// on considère une intersection vide si x == 0 && y == 0
	if(inter->position->posX == 0 && inter->position->posY == 0)
	{
		return true;
	}
	return false;
}

Intersection** creerTableInter() 
{ 
	float posX = MARGE_FEN; // positionnement en x
	float posY = MARGE_FEN; // positionnement en y
	float saut = getCoteCase(); // distance à parcourir en deux intersections
	
	Intersection** lesInters = malloc(dims_plateau*dims_plateau*sizeof(Intersection*));
	
	// on parcourt toutes les intersections...
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			lesInters[i * dims_plateau + j] = nouvIntersection(nouvCoord(posX, posY)); // ... et on les stocke dans le tableau
			setTypeIntersection(lesInters[i * dims_plateau + j]); // on attribue le type de l'intersection (bord, coin etc..)
			setNbLibertes(lesInters[i * dims_plateau + j]); // on attribue les libertés
			posX += getCoteCase(); // prochaine postion en x à traiter
		}
		posX = MARGE_FEN;
		posY += saut; // prochaine postion en y à traiter
	}
	
	//~ for (int i = 0; i < dims_plateau; i++) 
	//~ {
		//~ for (int j = 0; j < dims_plateau; j++)
		//~ {
			//~ setFils(lesInters[i * dims_plateau + j]);  // On définit les intersections adjacentes (les fils)
		//~ }
	//~ }
	return lesInters;
}

bool estCoin(Intersection* inter)
{	
	
	// Si l'intersection est l'un des coins, on retourne vrai
	if(inter->type == COIN_HG || inter->type == COIN_HD || inter->type == COIN_BG || inter->type == COIN_BD)
	{
		return true;
	}
	return false;
}

bool estBordure(Intersection* inter)
{
	
	// Si l'intersection est l'un des bords, on retourne vrai
	if (inter->type == BORD_HAUT || inter->type == BORD_DROIT || inter->type == BORD_BAS || inter->type == BORD_GAUCHE)
	{
		return true;
	}
	return false;
}

void setNbLibertes(Intersection* inter)
{
	// Si l'intersection est un coin, on mets les libertées à 2
	if(estCoin (inter))
	{
		inter->nbLibertes = 2;
	}
	
	// Si l'intersection est un bord, on mets les libertées à 3
	else if(estBordure(inter))
	{
		inter->nbLibertes = 3;
	}
	
	// Sinon, c'est une intersection du milieu donc avec 4 libertés
	else
	{
		inter->nbLibertes = 4;
	}
}

void setTypeIntersection (Intersection* inter)
{
	
	// coin haut gauche
	if((inter->position->posX == MARGE_FEN) && (inter->position->posY == MARGE_FEN))
	{
		inter->type = COIN_HG;
	}
	
	// coin haut droite
	else if((inter->position->posX == width_win() - MARGE_FEN) && (inter->position->posY == MARGE_FEN))
	{
		inter->type = COIN_HD;
	}
	
	// coin bas gauche
	else if((inter->position->posX == MARGE_FEN) && (inter->position->posY == height_win() - MARGE_FEN))
	{
		inter->type = COIN_BG;
	}
	
	// coin bas droite
	else if((inter->position->posX == width_win() - MARGE_FEN) && (inter->position->posY == height_win() - MARGE_FEN))
	{
		inter->type = COIN_BD;
	}
	
	// Bord gauche
	else if(inter->position->posX == MARGE_FEN)
	{
		inter->type = BORD_GAUCHE;	
	}
	
	//  Bord droite
	else if(inter->position->posX == width_win() - MARGE_FEN)
	{
		inter->type = BORD_DROIT;		
	}
	 
	// Bord Haut
	else if (inter->position->posY == MARGE_FEN)
	{
		inter->type = BORD_HAUT;	
	} 
	
	// Bord bas
	else if(inter->position->posY == height_win() - MARGE_FEN)
	{
		inter->type = BORD_BAS;
	}
	else 
	{
		inter->type = DEFAUT;	
	}
}

Intersection* getIntersectionHaut(Intersection* inter)
{
	Intersection* interTest = NULL;
	// Si l'intersection ne fait pas partie de la bordure ou des coins du haut, on retourne l'intersection du haut..
	if(inter->type != BORD_HAUT && inter->type != COIN_HG && inter->type != COIN_HD)
	{
		return getIntersection(inter->position->posX, inter->position->posY - getCoteCase());
	}
	//..sinon on retourne une valeure nulle
	return interTest;
}

Intersection* getIntersectionDroite(Intersection* inter)
{
	Intersection* interTest = NULL;
	// Si l'intersection ne fait pas partie de la bordure ou des coins de la droite, on retourne l'intersection de la droite..
	if(inter->type != BORD_DROIT && inter->type != COIN_HD && inter->type != COIN_BD)
	{
		return getIntersection(inter->position->posX + getCoteCase(), inter->position->posY);
	}
	//..sinon on retourne une valeure nulle
	return interTest;
}

Intersection* getIntersectionBas(Intersection* inter)
{
	Intersection* interTest = NULL;
	// Si l'intersection ne fait pas partie de la bordure ou des coins du bas, on retourne l'intersection du bas..
	if(inter->type != BORD_BAS && inter->type != COIN_BG && inter->type != COIN_BD)
	{
		return getIntersection(inter->position->posX, inter->position->posY + getCoteCase());
	}
	//..sinon on retourne une valeure nulle
	return interTest;
}

Intersection* getIntersectionGauche(Intersection* inter)
{
	Intersection* interTest = NULL;
	// Si l'intersection ne fait pas partie de la bordure ou des coins de la gauche, on retourne l'intersection de la gauche..
	if(inter->type != BORD_GAUCHE && inter->type != COIN_HG && inter->type != COIN_BG)
	{
		return getIntersection(inter->position->posX - getCoteCase(), inter->position->posY);
	}
	//..sinon on retourne une valeure nulle
	return interTest;
}

void incrementChaine(Chaine* chaine, Intersection* pierre)
{
	pierre->suiteChaine = NULL; // la pierre posée n'a pas de suivant
	chaine->finChaine->suiteChaine = pierre; // la suite du dernier élément de la chaîne à laquelle on ajoute est désormais la pierre posée
	chaine->finChaine = pierre; // le dernier élément de ladite chaîne devient la pierre ajoutée
	pierre->chMere = chaine; // la pierre ajoutée appartient à la chaîne
	chaine->nbPierres++; // on incrémente le nombre d'éléments de la chaîne
	printf("chaine incrémentée");
}

bool estMemeCouleur(Intersection* inter1, Intersection* inter2)
{
	if(inter1->couleur && inter2->couleur)
		{
		if(inter1->couleur == inter2->couleur)
		{
			return true;
		}
	}
	return false;
}

void initChaine(Intersection* inter)
{
	Chaine* c = malloc(sizeof(Chaine));
	inter->chMere = c;
	c->debutChaine = inter;
	c->finChaine = inter;
	c->nbPierres = 1;
	lesChaines[nbChaines] = c;
	nbChaines++;
}

void printChaines() 
{
	for(int i = 0; i < nbChaines; i++)
	{
		printf("%p\n", lesChaines[i]);
	}
}

void checkLesAdjacents(Intersection* inter)
{
	if (estNouvChaine(inter) == false)
	{
		
		if(getIntersectionHaut(inter) && getIntersectionHaut(inter)->estOccupe == true)
		{
			printf("InterHautOccupe\n");
			if(estMemeCouleur(inter, getIntersectionHaut(inter)))
			{
				incrementChaine(getIntersectionHaut(inter)->chMere, inter);
			}
		}
		
		if(getIntersectionDroite(inter) && getIntersectionDroite(inter)->estOccupe == true)
		{
			printf("InterDroiteOccupe\n");
			if(estMemeCouleur(inter, getIntersectionDroite(inter)))
			{
				incrementChaine(getIntersectionDroite(inter)->chMere, inter);
			}
		}
		
		if(getIntersectionBas(inter) && getIntersectionBas(inter)->estOccupe == true)
		{
			printf("InterBasOccupe\n");
			if(estMemeCouleur(inter, getIntersectionBas(inter)))
			{
				incrementChaine(getIntersectionBas(inter)->chMere, inter);
			}
		}
		
		if(getIntersectionGauche(inter) && getIntersectionGauche(inter)->estOccupe == true)
		{
			printf("InterGaucheOccupe\n");
			if(estMemeCouleur(inter, getIntersectionGauche(inter)))
			{
				incrementChaine(getIntersectionGauche(inter)->chMere, inter);
			}
		}
	}
	else 
	{
		printf("Intersection initialisée \n");
		initChaine(inter);
	}
}

bool estNouvChaine(Intersection* inter)
{
	CouleurPierre couleurTest = inter->couleur; // couleur à vérifier
	int cptCoulDiff = 0;
	
	if (aAdjacentOccupe(inter) == true)
	{
		int nbAdjacents = getNbAdjacents(inter);
		Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);
		
		for(int i = 0; i < nbAdjacents; i++)
		{
			if(lesAdjacents[i]->couleur != couleurTest)
			{
				cptCoulDiff++;
			}
		}
		
		free(lesAdjacents);
		
		if(cptCoulDiff == nbAdjacents)
		{
			return true;
		}
	}
	else
	{
		return true;
	}
	
	return false;
}

Intersection** getLesAdjacents(Intersection* inter, int taille)
{
	Intersection** lesAdjacents = malloc(taille*sizeof(Intersection*));
	
	int i = 0;
	
	if(getIntersectionHaut(inter))
	{
		lesAdjacents[i] = getIntersectionHaut(inter);
		i++;
	}
	if(getIntersectionDroite(inter))
	{
		lesAdjacents[i] = getIntersectionDroite(inter);
		i++;
	}
	if(getIntersectionBas(inter))
	{
		lesAdjacents[i] = getIntersectionBas(inter);
		i++;
	}
	if(getIntersectionGauche(inter))
	{
		lesAdjacents[i] = getIntersectionGauche(inter);
		i++;
	}
	
	return lesAdjacents;
}

int getNbAdjacents(Intersection* inter)
{
	if(estCoin(inter))
	{
		return 2;
	}
	else if(estBordure(inter))
	{
		return 3;
	}
	else
	{
		return 4;
	}
}

bool aAdjacentOccupe(Intersection* inter)
{
	bool aAdjacentOccupe = false;
	
	int nbAdjacents = getNbAdjacents(inter);
	Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);
	
	for(int i = 0; i < nbAdjacents; i++)
	{
		if(lesAdjacents[i]->estOccupe == true)
		{
			aAdjacentOccupe = true;
		}
	}
	
	free(lesAdjacents);
	return aAdjacentOccupe;
}

void freeAll()
{
	// on parcourt toutes les intersections...
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			free(lesInters[i * dims_plateau + j]->position);
			free(lesInters[i * dims_plateau + j]);
		}
	}
	
	free(lesInters);
}


///////////////////////////////
// PARTIE GESTION DES REGLES //
///////////////////////////////



int main() 
{
	int coteFen = 18*COTE_CASE_19 + 2*MARGE_FEN;
	init_win(coteFen,coteFen, "Jeu de GO",0.988,0.807,0.611);
	printf("Quelles dimensions ? ");
	////////////////////////////////////////////////////////////////////
	// Si oublié les boutons, faire fonction qui check entrée clavier //
	///////////////////////////////////////////////////////////////////
	scanf("%d", &dims_plateau);
	lesInters = creerTableInter();
	lesChaines = malloc(dims_plateau * dims_plateau * sizeof(Chaine*));
	event_loop_plateau();
	//event_loop_niveau();
	printf("lollllllllllllllllllllllllllll");
	freeAll();
	return EXIT_SUCCESS;
}
