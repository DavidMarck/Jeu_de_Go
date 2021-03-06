#include "jeu.h"


const float COTE_CASE_19 = 30; // côté des cases plateau 19x19 lignes (18x18 cases)
const float RAYON_PIERRE_19 = 10; // rayon d'une pierre sur un plateau 19x19

const float COTE_CASE_13 = 45; // côté des cases plateau 13x13 lignes (12x12 cases) ==> 18/12 * COTE_CASE_19
const float RAYON_PIERRE_13 = 15; // rayon d'une pierre sur un plateau 13x13

const float COTE_CASE_9 = 67.5; // côté des cases plateau 9x9 lignes (8x8 cases) ==> 18/8 * COTE_CASE_19
const float RAYON_PIERRE_9 = 22.5; // rayon d'une pierre sur un plateau 9x9

const int MARGE_FEN = 45; // Marge du plateau par rapport à la fenêtre

const int COTE_FEN = 630; // 18*COTE_CASE_19 + 2*MARGE_FEN; à modifier quand la marge change

static int dims_plateau; // dimensions du plateau choisies (19, 13 ou 9)
static Intersection** lesInters; // tableau de structures contenant les intersections du goban (plateau)
static Chaine** lesChaines;	// tableau de structures contenant les chaines du goban au fil de la partie(plateau)
static int nbChaines = 0; // Valeur permettant de parcourir 

static CouleurPierre tour = NOIR;  // indique le tour actuel
static Mode modeJeu = MENU;  // prend en compte les hitbox du menu

/**
 * Mettre ici son code pour dessiner dans la fenetre (plateau)
 * 1er affichage + redessine si resize
 */
void draw_win_plateau()
{	
	// vide la fenetre
	clear_win();

	float coteCase = getCoteCase(); 	// on récupère la longueur du côté des cases en fonction de la dimension du plateau
	color(1, 0, 0);  // On prend une couleur rouge pour les cases
	rectangle(MARGE_FEN,MARGE_FEN,(dims_plateau-1)*coteCase, (dims_plateau-1)*coteCase);	// Trace le rectangle qui fait le contour du plateau
	
	dessine_plateau(); // Dessine le plateau du jeu (en fonction des dimensions choisies dans le menu)
	dessine_hoshi(); // met en place les hoshis du plateau
	
	//~ color(0.5,0.5,0.5);
	//~ filled_rectangle(0, height_win() - 20, 100, 20);
	
	//~ color(1, 1, 1);
	//~ string(5,(height_win() - 5),"RETOUR AU MENU");
	
	// Couleur de la première pierre qui sera posée (noir)
	color(0, 0, 0);
}


void draw_win_menu()
{	
	clear_win();
	
	color(0.5,0.5,0.5); // Couleur grise
	
	filled_rectangle(MARGE_FEN*5,MARGE_FEN,width_win() - 10*MARGE_FEN,90);		// Le bouton pour le plateau 19x19
	filled_rectangle(MARGE_FEN*5,2*MARGE_FEN + 90,width_win() - 10*MARGE_FEN,90);  // Le bouton pour le plateau 13x13
	filled_rectangle(MARGE_FEN*5,3*MARGE_FEN + 2*90,width_win() - 10*MARGE_FEN,90);	// Le bouton pour le plateau 9x9
	
	color (1,1,1); // Couleurblanche
	
	//on installe les string sur les boutons
	string(width_win()/2 - 15,(MARGE_FEN + 45),"19x19");
	string(width_win()/2 - 15,(MARGE_FEN + 45)+(MARGE_FEN+90),"13x13");
	string(width_win()/2 - 15,(MARGE_FEN + 45)+2*(MARGE_FEN+90),"9x9");
	
}


/**
 * on a cliqué a la souris (plateau):
 * bouton: 1,2,3,4,5,... : gauche, milieu, droit, molette, ...
 * x,y position
 * dim dimension du plateau en cours
 */
void mouse_clicked_plateau(int bouton, int x, int y)
{
	switch (modeJeu)
	{
		case MENU :		// Si on est sur la fenêtre de menu, on définit les coordonnées des boutons sur lesquels on peut cliquer pour lancer une partie
		
			if (x >= MARGE_FEN*5 && x <= width_win() - 5*MARGE_FEN)
			{
				if(y >= MARGE_FEN && y <= MARGE_FEN + 90)
				{
					dims_plateau = 19;
					lesInters = creerTableInter();
					lesChaines = malloc(dims_plateau * dims_plateau * sizeof(Chaine*));
					draw_win_plateau();
					modeJeu = JEU;
				}
				else if (y >= (2*MARGE_FEN + 90) && y <= (2*MARGE_FEN + 2*90))
				{
					dims_plateau = 13;
					lesInters = creerTableInter();
					lesChaines = malloc(dims_plateau * dims_plateau * sizeof(Chaine*));
					draw_win_plateau();
					modeJeu = JEU;
				}
				else if ( y >= (3*MARGE_FEN + 2*90) &&  y <= (3*MARGE_FEN + 3*90))
				{
					dims_plateau = 9;
					lesInters = creerTableInter();
					lesChaines = malloc(dims_plateau * dims_plateau * sizeof(Chaine*));
					draw_win_plateau();
					modeJeu = JEU;
				}
			}
			break;
		case JEU :
		
			//~ if ( x >= 0 && x<= 100 && y >= height_win() - 20 && y <= height_win())
			//~ {
				//~ freeAll();
				//~ draw_win_menu();
				//~ modeJeu = Menu;
			//~ }
			//~ else 
			//~ {
			
			printf("Bouton %d presse au coord. %d,%d \n",bouton,x,y);

			Intersection* inter = getIntersection(x, y);
			
			if(coupEstPermis(inter)) // Si le coup est permis
			{
				filled_circle(inter->position->posX,inter->position->posY,getRayonPierre()); // ... on remplit l'intersection...
				inter->estOccupe = true; // ... on définit que l'intersection est maintement occupée...
				inter->couleur = tour; // On adapte la couleur au joueur
				chainage(inter);  // On gère la création/fusion de chaines
				updateNbLibertes(); // On met à jour les libertés du plateau
				updatePlateau(); // On met à jour le plateau (redessine)
				changerTour(); // ... et enfin on passe au tour suivant
			}
			break;	
	}
	
}

void updateLibertesChaines()
{
	// On parcourt toutes les chaines en jeu...
	for (int i = 0; i < nbChaines; i++) 
	{
		// Si une chaine n'a plus de liberté, on parcourt toutes les pierres(Intersection) qu'elles contient pour les vider
		if(getNbLibertesChaine(lesChaines[i]) == 0) 
		{
			Intersection* interParcours = lesChaines[i]->debutChaine;  // permet de parcourir la chaine
			
			do
			{
				int nbAdjacents = getNbAdjacents(interParcours);	// On récupère le nombre intersection adjacentes à l'intersection ciblée...
				Intersection** lesAdjacents = getLesAdjacents(interParcours, nbAdjacents); //.. et les cases adjacentes elles-mêmes
				
				// On rend une liberté aux intersections adjacentes
				for (int i = 0; i < nbAdjacents; i++)
				{
					lesAdjacents[i]->nbLibertes++;
				}
				
				interParcours->estOccupe = false;  // On indique que l'intersection n'est plus occupée par une pierre...
				interParcours->chMere = NULL; //.., qu'elle n'appartient plus à une chaine...
				interParcours->couleur = 0;	//.., qu'elle n'a plus de couleur..
				interParcours = interParcours->suiteChaine;	//..et on passe à l'intersection suivante de la chaine
			} while(interParcours != NULL);
			
			
			supprimeChaine(lesChaines[i]);
		}
	}
}

void updatePlateau ()
{
	updateLibertesChaines();  // Met à jour les libertés des chaines
	draw_win_plateau();	// Redessine le plateau (pour effacer toute les pierres)
	dessine_Pierres();	// Dessine les pierres qui sont en encore en jeu
}

void updateNbLibertes() 
{
	// Pour chaque intersection du plateau...
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			int nbAdjacents = getNbAdjacents(lesInters[i * dims_plateau + j]);	// On récupère le nombre intersection adjacentes à l'intersection ciblée...
			Intersection** lesAdjacents = getLesAdjacents(lesInters[i * dims_plateau + j],nbAdjacents); //.. et les cases adjacentes elles-mêmes
			
			int nbAdjOcc = 0; // On initialise le nombre d'occurences occupées par une pierre
			
			// pour chaque intersection adjacente...
			for(int k = 0; k < nbAdjacents; k++)
			{	
				// Si l'intersection est occupée par une pierre, on incrémente les occurences
				if(lesAdjacents[k]->estOccupe == true)
				{
					nbAdjOcc++;
				}
			}
			
			lesInters[i * dims_plateau + j]->nbLibertes = getNbLibertesTotal(lesInters[i * dims_plateau + j]) - nbAdjOcc; // L'intersection ciblée récupère les libertés
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
	bool coupEstPermis = false;
	
	bool estVide = interEstVide(inter); // Si l'intersection est vide (x=0;y=0), alors c'est que l'utilisateur a cliqué en dehors des intersections cliquables
	bool estOccupe = inter->estOccupe; // On vérifie que l'intersection concernée ne soit pas occupée
	int nbLibertes = inter->nbLibertes;
	
	if(estVide == false && estOccupe == false) // S'il s'agit bien d'une intersection du plateau (non vide), et qu'elle n'est pas occupée...
	{
		if(nbLibertes > 0) // Si il y a au moins une liberté, on peut poser
		{
			coupEstPermis = true;
		}
		else // Sinon si il n'y aplus de liberté
		{	
			// On appelle les intersection adjacentes à celle ciblée
			int nbAdjacents = getNbAdjacents(inter);
			Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);
			
			// Pour chaque intersection adjacente
			for(int i = 0; i < nbAdjacents; i++)
			{
				// Si une intersection adjacente n'est pas de la même couleur que celle de la pierre que l'on veut poser...
				if(lesAdjacents[i]->couleur != tour) 
				{
					if(getNbLibertesChaine(lesAdjacents[i]->chMere) == 1) //..et qu'elle sera capturée par la pose, on autorise la pose
					{
						coupEstPermis = true;
						break;
					}
				}
			}
			free(lesAdjacents);
		}
	}
	
	return coupEstPermis;
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
	//~   case XK_Control_R:
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
	// Même chose de haut en basl'intersection qui va virer : 0x5d816f0

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

void dessine_Pierres()
{
	// On parcourt toutes les intersections du plateau
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			//	Si une intersection est occupée..
			if(lesInters[i * dims_plateau + j]->estOccupe == true)
			{
			
			//... On adapte la couleur de la pierre occupante
			switch (lesInters[i * dims_plateau + j]->couleur) 
				{
					case NOIR :
						color (0, 0, 0);
						break;
					
					case BLANC :
						color(1, 1, 1);
						break;
						
					default :
						break;
				}
				filled_circle(lesInters[i * dims_plateau + j]->position->posX, lesInters[i * dims_plateau + j]->position->posY, getRayonPierre()); //	On trace le cercle
			}
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
	
	inter->position = coord;	// On donne à l'intersection les coordonnées en entrées
	inter->estOccupe = false;	// Par défaut une intersection n'est pas occupée..
	inter->chMere = NULL;		//..elle n'appartient à aucune chaine..
	inter->suiteChaine = NULL;	//..et n'a donc pas de suite
	
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
			lesInters[i * dims_plateau + j]->chMere = NULL;
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

int getNbLibertesTotal(Intersection* inter)
{			
	if(estCoin(inter))
	{
		return 2;
	} 
	else if (estBordure(inter))
	{
		return 3;
	}
	
	return 4;
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
	// Autres
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
	if(pierre->chMere == NULL)
	{
		pierre->chMere = chaine; // La pierre ajoutée appartient maintenant à la chaîne
		pierre->suiteChaine = NULL; // La pierre posée n'a pas de suivant
		chaine->finChaine->suiteChaine = pierre; // la suite du dernier élément de la chaîne à laquelle on ajoute est désormais la pierre posée
		chaine->finChaine = pierre; // Le dernier élément de ladite chaîne devient la pierre ajoutée
		chaine->nbPierres++; // On incrémente le nombre d'éléments de la chaîne
			}
	// Si la pierre possée appartient déjà à une chaîne (cas où plusieurs chaînes adjacentes)
	// et que celle de l'intersection adjacente en cours de vérification n'est pas déjà la même
	else if(chaine != pierre->chMere)
	{
		Intersection* parcoursChaine = chaine->debutChaine;	// Variable qui parcourt la chaine
		
		// On parcourt la chaine de l'intersecion pour leur donner comme chaine celle de la pierre posée
		do
		{
			parcoursChaine->chMere = pierre->chMere;
			parcoursChaine = parcoursChaine->suiteChaine;
		}
		while(parcoursChaine);

		pierre->chMere->finChaine->suiteChaine = chaine->debutChaine; // On acolle la nouvelle chaine croisée à celle parente à la pierre posée
		pierre->chMere->finChaine = chaine->finChaine; // On actualise le dernier élément
		pierre->chMere->nbPierres += chaine->nbPierres; // On augmente le nombre de pierres dans la chaine qui va rester
		
		supprimeChaine(chaine); // On libère la chaîne anciennement adjacente
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
	
	Chaine* c = malloc(sizeof(Chaine));	// On crée une chaine avec allocation
	inter->chMere = c;	// On indique à l'intersection qu'elle appartient à cette chaine
	c->debutChaine = inter;	// On indique à la chaine que l'intersection est son point de départ..
	c->finChaine = inter;	//..et son point de fin
	c->nbPierres = 1;	// On fixe le nombre de pierres à 1
	lesChaines[nbChaines] = c;	// On ajoute la nouvelle chaine au tableau des chaines du jeu
	nbChaines++;	// On incrémente le nombre total de chaine
}

void chainage(Intersection* inter)
{
	// Si une chaine n'est pas à initialiser (car adjacente à une autre de la même couleur)...
	if (estNouvChaine(inter) == false)
	{
		
		int nbAdjacents = getNbAdjacents(inter);						  // On récupère le nombre d'adjacent
		Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents); // On récupère les intersections adjacentes à l'intersection 
		
		// Pour chaque case adjacentes..
		for(int i = 0; i < nbAdjacents; i++)
		{
			// Si l'intersection adjacente est occupée par une pierre de la même couleur que l'intersection cible, on incrémente la chaine
			if((lesAdjacents[i]->estOccupe == true) && (estMemeCouleur(inter, lesAdjacents[i])))
			{
				incrementChaine(lesAdjacents[i]->chMere, inter);
			}
		}
		free(lesAdjacents);
	}
	else 
	{
		initChaine(inter); // On place l'intersection dans une nouvelle chaine
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

Intersection** getLesLibertes(Intersection* inter)
{
	int nbLibertes = inter->nbLibertes;  // nombres de libertés actuelles à l'intersection

	Intersection** lesLibertes = malloc(nbLibertes*sizeof(Intersection*)); // On crée un tableau qui contiendra les libertés(Intersections*) de l'intersection
	int indexLiberte = 0; // On initialise un compteur
	
	int nbAdjacents = getNbAdjacents(inter);
	Intersection** lesAdjacents = getLesAdjacents(inter,nbAdjacents);
	
	// Pour chaque intersection adjacente à la chaine cible...
	for(int i = 0; i < nbAdjacents; i++)
	{
		// ... si l'intersection adjacente n'est pas occupée, on ajoute la liberté correspondante (le nombre étant symbolisé par le compteur)
		if(!lesAdjacents[i]->estOccupe)
		{
			lesLibertes[indexLiberte] = lesAdjacents[i];
			indexLiberte++;
		}
	}
	
	free(lesAdjacents);
	return lesLibertes;
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
	int nbLibChaine = 0;
	Intersection* inter = chaine->debutChaine;
	
	Intersection** lesLibertesInter;	// Tableau contenant les liberté que l'on compte (pour ne pas en compter une deux fois si elle est commune à deux intersections)
	Intersection** lesLibertesChaine = malloc(dims_plateau*dims_plateau*sizeof(Intersection*));
	
	bool libDejaListee = false; // Sert à indiquer si une liberté se a déjà été comptée (dans le cas ou des intersections ont des libertés communes)
	
	// On parcourt la liste chainée dans son intégralité
	do
	{
		lesLibertesInter = getLesLibertes(inter); // On récupère les intersections adjacentes qui sont libres (les libertés)
		
		// Pour chaque liberté...
		for(int i = 0; i < inter->nbLibertes; i++)
		{
			libDejaListee = false;	// On initie le vérificateur à faux
			
			//	Si on a au moins une libertée stockée dans le tableau qui les mémorise...
			if(lesLibertesChaine != NULL) 
			{
				// On parcourt les libertés
				for (int j = 0; j < nbLibChaine; j++) 
				{
					// Si la liberté se trouve déjà dans le tableau qui contient celles stockées
					if(lesLibertesChaine[j] == lesLibertesInter[i])
					{
						libDejaListee = true; // On précise mes le vérificateur à vrai pour ne pas la compter
						break;
					}
				}
				
				// Si la liberté n'a pas été comptée auparavent, on l'ajoute au tableau qui les mémorise et on incrémente les libertés
				if(libDejaListee == false)
				{
					lesLibertesChaine[nbLibChaine] = lesLibertesInter[i];
					nbLibChaine++;
				}
			}
			// Sinon on incrémente et on place la liberté dans la première case
			else
			{
				lesLibertesChaine[0] = lesLibertesInter[i];
				nbLibChaine++;
			}
		}
		inter = inter->suiteChaine;
	} while (inter != NULL);
	
	free(lesLibertesInter);
	free(lesLibertesChaine);
	return nbLibChaine;
}

void supprimeChaine(Chaine* chaine)
{
	// On parcourt le tableau contenant les chaines du jeu
	for(int i = 0; i < nbChaines; i++)
	{
		// Si la chaine correspond à celle en entrée, on la supprime du tableau tout dans décallant les chaines suivantes
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
	// on parcourt tout pour libérer la mémoire
	for (int i = 0; i < dims_plateau; i++) 
	{
		for (int j = 0; j < dims_plateau; j++)
		{
			
			free(lesInters[i * dims_plateau + j]->position);
			free(lesInters[i * dims_plateau + j]->suiteChaine);
			free(lesInters[i * dims_plateau + j]->chMere);
			lesInters[i * dims_plateau + j] = NULL;
			free(lesInters[i * dims_plateau + j]);
			
			if (lesChaines[i * dims_plateau + j] != NULL)
			{
				free(lesChaines[i * dims_plateau + j]->debutChaine);
				free(lesChaines[i * dims_plateau + j]->finChaine);
				free(lesChaines[i * dims_plateau + j]);
			}
		}
	}
	
	free(lesInters);
	free(lesChaines);
}

int main() 
{
	init_win(COTE_FEN,COTE_FEN, "Jeu de GO",0.988,0.807,0.611);
	event_loop_plateau();
	return EXIT_SUCCESS;
}
