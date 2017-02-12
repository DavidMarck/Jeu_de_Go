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
	
	if(coupEstPermis(inter)) // Si le coup est permis
	{
		filled_circle(inter->position->posX,inter->position->posY,getRayonPierre()); // ... on la remplit...
		inter->estOccupe = true; // ... on définit que l'intersection est maintement occupée...
		inter->couleur = tour;
		checkLesAdjacents(inter);
		updateNbLibertes();
		printChaines();
		changerTour(); // ... et enfin on passe au tour suivant
	}
}

void updateNbLibertes() 
{
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			int nbAdjacents = getNbAdjacents(lesInters[i * dims_plateau + j]);
			Intersection** lesAdjacents = getLesAdjacents(lesInters[i * dims_plateau + j],nbAdjacents);
			
			for(int k = 0; k < nbAdjacents; k++)
			{
				if(lesAdjacents[k]->estOccupe == true)
				{
					printf("%d\n",lesInters[i * dims_plateau + j]->nbLibertes);
					lesInters[i * dims_plateau + j]->nbLibertes--;
					printf("%d\n",lesInters[i * dims_plateau + j]->nbLibertes);
				}
			}
			
			free(lesAdjacents);
		}
	}
}

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
	// Si la pierre posée n'appartient pas encore à une chaîne
	if(!pierre->chMere)
	{
		pierre->chMere = chaine; // la pierre ajoutée appartient maintenant à la chaîne
		pierre->suiteChaine = NULL; // la pierre posée n'a pas de suivant
		chaine->finChaine->suiteChaine = pierre; // la suite du dernier élément de la chaîne à laquelle on ajoute est désormais la pierre posée
		chaine->finChaine = pierre; // le dernier élément de ladite chaîne devient la pierre ajoutée
		chaine->nbPierres++; // on incrémente le nombre d'éléments de la chaîne
		printf("chaine incrémentée\n");
	}
	// Si la pierre possée appartient déjà à une chaîne (cas où plusieurs chaînes adjacentes)
	// et que celle de l'intersection adjacente en cours de vérification n'est pas déjà la même
	else if(chaine != pierre->chMere)
	{
		chaine->debutChaine->chMere = pierre->chMere; // on ajoute la chaîne de la pierre adjacente à la chaîne de la pierre posée
		pierre->suiteChaine = chaine->debutChaine; // on lie les deux chaînes
		pierre->chMere->finChaine = chaine->finChaine; // on actualise le dernier élément
		
		supprimeChaine(chaine); // on libère la chaîne anciennement adjacente
	}
}

bool estMemeCouleur(Intersection* inter1, Intersection* inter2)
{
	// Si les deux intersections entrées ont la même couleur, on retourne vrai
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
	Chaine* c = malloc(sizeof(Chaine));	// on crée une chaine avec allocation
	inter->chMere = c;	// on indique à l'intersection qu'elle appartient à cette chaine
	c->debutChaine = inter;	// on indique à la chaine que l'intersection est son point de départ..
	c->finChaine = inter;	//.. et son point de fin
	c->nbPierres = 1;	// on fixe le nombre de pierres à 1
	lesChaines[nbChaines] = c;	// On ajoute la nouvelle chaine au tableau des chaines du jeu
	nbChaines++;	// On incrémente le nombre total de chaine
}

void printChaines() 
{
	for(int i = 0; i < nbChaines; i++)
	{
		printf("%p; libertés = %d\n", lesChaines[i], getNbLibertesChaine(lesChaines[i]));
	}
}

void checkLesAdjacents(Intersection* inter)
{
	// Si une chaine n'est pas à initialiser (car adjacente à une autre de la même couleur)...
	if (estNouvChaine(inter) == false)
	{
		
		int nbAdjacents = getNbAdjacents(inter);
		Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);
		
		for(int i = 0; i < nbAdjacents; i++)
		{
			if((lesAdjacents[i]->estOccupe == true) && (estMemeCouleur(inter, lesAdjacents[i])))
			{
				incrementChaine(lesAdjacents[i]->chMere, inter);
			}
		}
		free(lesAdjacents);
	}
	else 
	{
		printf("Nouvelle chaîne \n");
		initChaine(inter);
	}
}

bool estNouvChaine(Intersection* inter)
{
	CouleurPierre couleurTest = inter->couleur; // couleur à vérifier
	int cptCoulDiff = 0;	// On initialise le nombre de couleur qui divergent de celle de l'intersection
	
	// si l'intersection à au moins une intersection adjacente qui est occupée...
	if (aAdjacentOccupe(inter) == true)
	{
		
		int nbAdjacents = getNbAdjacents(inter);  // On récupère le nombre d'adjacents..
		Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);	//.. et les adjacents eux-mêmes
		
		// Pour chaque Adjacent...
		for(int i = 0; i < nbAdjacents; i++)
		{
			//.. si sa couleur est différente de la pierre posée diffère de celle de la case adjacente, on incrémente le nombre de couleur divergent
			if(lesAdjacents[i]->couleur != couleurTest)
			{
				cptCoulDiff++;
			}
		}
		
		free(lesAdjacents); 
		
		// Si toutes les cases occupées adjacentes sont de couleur différente, on retourne vrai
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
	Intersection** lesAdjacents = malloc(taille*sizeof(Intersection*)); // On crée un tableau qui contiendra les intersections adjacentes
	
	int i = 0; // i fait office de compteur qui s'incrémente pour remplir le tableau
	
	// si  une intersection se trouve en haut on l'ajoute au tableau
	if(getIntersectionHaut(inter))
	{
		lesAdjacents[i] = getIntersectionHaut(inter);
		i++;
	}
	
	// si  une intersection se trouve à droite on l'ajoute au tableau
	if(getIntersectionDroite(inter))
	{
		lesAdjacents[i] = getIntersectionDroite(inter);
		i++;
	}
	
	// si  une intersection se trouve en bas on l'ajoute au tableau
	if(getIntersectionBas(inter))
	{
		lesAdjacents[i] = getIntersectionBas(inter);
		i++;
	}
	
	// si une intersection se trouve à gauche on l'ajoute au tableau
	if(getIntersectionGauche(inter))
	{
		lesAdjacents[i] = getIntersectionGauche(inter);
		i++;
	}
	
	return lesAdjacents;
}

int getNbAdjacents(Intersection* inter)
{
	// Si c'est un coin, on a 2 intersections adjacentes
	if(estCoin(inter))
	{
		return 2;
	}
	// Si c'est une bordure, on a 3 intersections adjacentes
	else if(estBordure(inter))
	{
		return 3;
	}
	// Sinon, on en a 4
	else
	{
		return 4;
	}
}

bool aAdjacentOccupe(Intersection* inter)
{
	bool aAdjacentOccupe = false;
	
	int nbAdjacents = getNbAdjacents(inter); // On répupère le nombre d'intersections adjacentes
	Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents); // On récupère ces mêmes intersections dans un tableau
	
	// Pour chaque intersection adjacente...
	for(int i = 0; i < nbAdjacents; i++)
	{
		//.. si l'intersection est occupé par une pierre on retourne true
		if(lesAdjacents[i]->estOccupe == true)
		{
			aAdjacentOccupe = true;
		}
	}
	
	free(lesAdjacents);  //On libère la mémoire
	return aAdjacentOccupe;
}

int getNbLibertesChaine(Chaine* chaine)
{
	int nbTotLibertes = 0;
	Intersection* inter = chaine->debutChaine;
	
	do
	{
		nbTotLibertes += inter->nbLibertes;
		inter = inter->suiteChaine;
	} while (inter);
	
	return nbTotLibertes;
}

void supprimeChaine(Chaine* chaine)
{
	for(int i = 0; i < nbChaines; i++)
	{
		if(lesChaines[i] == chaine)
		{
			if(i != nbChaines - 1)
			{
				int j = i;
				while(j != nbChaines - 1)
				{
					lesChaines[j] = lesChaines[j+1];
					j++;
				}
				lesChaines[nbChaines-1] = NULL;
			}
			else
			{
				lesChaines[i] = NULL;
			}
		}
	}
	nbChaines--;
	free(chaine);
}

void freeAll()
{
	// on parcourt toutes les intersections...
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			free(lesInters[i * dims_plateau + j]->position);
			free(lesInters[i * dims_plateau + j]->suiteChaine);
			free(lesInters[i * dims_plateau + j]->chMere);
			free(lesInters[i * dims_plateau + j]);
			
			free(lesChaines[i * dims_plateau + j]->debutChaine);
			free(lesChaines[i * dims_plateau + j]->finChaine);
			free(lesChaines[i * dims_plateau + j]);
		}
	}
	
	free(lesInters);
	free(lesChaines);
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
