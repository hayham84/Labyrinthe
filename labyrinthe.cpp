#include <ncurses.h>
#include "labIO.h"
#include<iostream>
#include<fstream>
#include <cstdlib>
#include<unistd.h>

using namespace std;

void init(Case & c)
{
    // Initialise une case avec toutes les portes fermées et non visitée
    c.N = c.S = c.E = c.W = false;
    c.etat = false;
}

void init(laby & L, int p, int q)
{
    // Initialise la structure du labyrinthe avec p lignes et q colonnes
    L.p = p; L.q = q;
    L.tab = new Case*[p];
    for (int i = 0; i < p; i++) L.tab[i] = new Case[q];
    for (int i = 0; i < p; i++)
        for (int j = 0; j < q; j++) init(L.tab[i][j]);
}

laby *lire_Laby(const char *filename)
{
    // Lit un labyrinthe depuis un fichier
    ifstream my_cin(filename);
    laby *L = new laby;
    my_cin >> L->p >> L->q;
    init(*L, L->p, L->q);
    for (int i = 0; i < L->p; i++)
        for (int j = 0; j < L->q; j++)
            my_cin >> L->tab[i][j].W >> L->tab[i][j].N >> L->tab[i][j].S >> L->tab[i][j].E;
    return L;
}

void ecrire_Laby(laby & L, const char *fn)
{
    // Sauvegarde un labyrinthe dans un fichier
    ofstream my_cout(fn);
    my_cout << L.p << " " << L.q << " ";
    for (int i = 0; i < L.p; i++)
        for (int j = 0; j < L.q; j++)
            my_cout << L.tab[i][j].W << " " << L.tab[i][j].N << " " << L.tab[i][j].S << " " << L.tab[i][j].E << " ";
}

void deplacement(laby & L, bool &abandon)
{
    // Gère les déplacements du joueur à travers le labyrinthe avec ncurses
    int i = 0, j = 0;
    bool game = true;
    Mark_Case(i, j); // Marque la case de départ

    while ((i != L.p - 1 || j != L.q - 1) && game)
    {
        switch (LireCommande())
        {
            case 9:  if (L.tab[i][j].W) { UMark_Case(i,j); j--; Mark_Case(i,j); } break; // Gauche
            case 3:  if (L.tab[i][j].E) { UMark_Case(i,j); j++; Mark_Case(i,j); } break; // Droite
            case 12: if (L.tab[i][j].N) { UMark_Case(i,j); i--; Mark_Case(i,j); } break; // Haut
            case 6:  if (L.tab[i][j].S) { UMark_Case(i,j); i++; Mark_Case(i,j); } break; // Bas
            case -1: clear(); abandon = true; game = false; break; // Abandon (F1)
            default:
                clear();
                mvprintw(0, 0, "echec : commande invalide");
                LireCommande(); EndCurses(); game = false;
                break;
        }
    }

    if (i == L.p - 1 && j == L.q - 1)
    {
        // Fin du labyrinthe atteinte
        clear();
        mvprintw(0, 0, "Bien joué !");
        LireCommande(); EndCurses();
    }
}

void init(pile & p, int taille)
{
    // Initialise une pile de couples
    p.taille = taille;
    p.nbe = 0;
    p.T = new couple[taille];
}

bool vide(pile & p) {
    return p.nbe == 0;
}


bool empiler(pile & p, couple c)
{
    // Empile un couple si possible
    if (p.nbe < p.taille) {
        p.T[p.nbe++] = c;
        return true;
    }
    return false;
}

couple depiler(pile & p)
{
    // Dépile le dernier couple
    if (p.nbe > 0) return p.T[--p.nbe];
    cout << "pile vide" << endl; exit(1);
}

bool adjacent_visite(laby L, couple C)
{
    // Vérifie si toutes les cases adjacentes à C ont été visitées
    if (C.i > 0 && !L.tab[C.i - 1][C.j].etat) return false;
    if (C.j < L.q - 1 && !L.tab[C.i][C.j + 1].etat) return false;
    if (C.i < L.p - 1 && !L.tab[C.i + 1][C.j].etat) return false;
    if (C.j > 0 && !L.tab[C.i][C.j - 1].etat) return false;
    return true;
}

laby * My_Creation(int p, int q)
{
    // Génère un labyrinthe parfait aléatoire en utilisant une pile et le backtracking
    laby *L = new laby;
    init(*L, p, q);

    couple current = {0, 0};
    pile cellStack;
    init(cellStack, p * q);
    empiler(cellStack, current);
    L->tab[0][0].etat = true;

    while (!vide(cellStack))
    {
        while (!adjacent_visite(*L, current))
        {
            int porte = rand() % 4 + 1;
            // Pour chaque direction possible, on vérifie si on peut y aller
            switch (porte)
            {
                case 1: // Nord
                    if (current.i > 0 && !L->tab[current.i - 1][current.j].etat) {
                        L->tab[current.i][current.j].N = true;
                        L->tab[current.i - 1][current.j].S = true;
                        empiler(cellStack, current);
                        current.i--; L->tab[current.i][current.j].etat = true;
                    }
                    break;
                case 2: // Est
                    if (current.j < q - 1 && !L->tab[current.i][current.j + 1].etat) {
                        L->tab[current.i][current.j].E = true;
                        L->tab[current.i][current.j + 1].W = true;
                        empiler(cellStack, current);
                        current.j++; L->tab[current.i][current.j].etat = true;
                    }
                    break;
                case 3: // Sud
                    if (current.i < p - 1 && !L->tab[current.i + 1][current.j].etat) {
                        L->tab[current.i][current.j].S = true;
                        L->tab[current.i + 1][current.j].N = true;
                        empiler(cellStack, current);
                        current.i++; L->tab[current.i][current.j].etat = true;
                    }
                    break;
                case 4: // Ouest
                    if (current.j > 0 && !L->tab[current.i][current.j - 1].etat) {
                        L->tab[current.i][current.j].W = true;
                        L->tab[current.i][current.j - 1].E = true;
                        empiler(cellStack, current);
                        current.j--; L->tab[current.i][current.j].etat = true;
                    }
                    break;
            }
        }
        current = depiler(cellStack);
    }

    return L;
}

void affiche_chemin(pile & P, laby & L)
{
    // Affiche le chemin en marquant chaque case depuis la pile
    for (int i = 0; i < P.taille; i++)
        Mark_Case(depiler(P).i, depiler(P).j);
}

bool adjacent_accessible(laby &L, couple *C)
{
    // Vérifie s’il existe une case adjacente accessible depuis la case actuelle
    if (C->i > 0 && !L.tab[C->i - 1][C->j].marq && L.tab[C->i][C->j].N) return true;
    if (C->j < L.q - 1 && !L.tab[C->i][C->j + 1].marq && L.tab[C->i][C->j].E) return true;
    if (C->i < L.p - 1 && !L.tab[C->i + 1][C->j].marq && L.tab[C->i][C->j].S) return true;
    if (C->j > 0 && !L.tab[C->i][C->j - 1].marq && L.tab[C->i][C->j].W) return true;
    return false;
}

pile *explorer(laby & L)
{
    // Exploration automatique du labyrinthe avec affichage progressif (ncurses)
    clear(); Show_Lab(L);
    couple *c = new couple;
    c->i = 0;
    c->j = 0;

    pile *chemin = new pile;
    init(*chemin, L.p * L.q);
    empiler(*chemin, *c);
    L.tab[0][0].marq = true;
    int taille_pile = 1;

    while (c->i != L.p - 1 || c->j != L.q - 1)
    {
        usleep(100000); refresh(); Show_Lab(L);
        if (adjacent_accessible(L, c))
        {
            bool found = false;
            while (!found)
            {
                int porte = rand() % 4 + 1;
                // Choix aléatoire d'une direction accessible et non encore visitée
                switch (porte){
                    case 1 :if (c->i > 0) { //N ==> est ce que la valeur de i ne représente pas une extrémitée (ici coté nord) du labyrinthe
                            if (!L.tab[c->i - 1][c->j].marq) { // est ce que la case adjacente est marqué ? ici la case au nord
                                if (L.tab[c->i][c->j].N) {//est ce que le mur qui sépare les deux case est bien ouvert ?
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    L.tab[c->i-1][c->j].marq = true;
                                    Mark_Case(c->i,c->j);
                                    c->i--;
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    found = true;
                                }
                            }
                        }
                        break;
                    case 2 : if (c->j < L.q-1){//E ==> meme raisonnement pour chaque mur
                            if (!L.tab[c->i][c->j+1].marq) {
                                if (L.tab[c->i][c->j].E) {
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    L.tab[c->i][c->j+1].marq = true;
                                    Mark_Case(c->i,c->j);
                                    c->j++;
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    found = true;
                                }
                            }
                        }
                        break;
                    case 3 : if (c->i < L.p-1) {//S
                            if (!L.tab[c->i + 1][c->j].marq) {
                                if (L.tab[c->i][c->j].S) {
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    L.tab[c->i + 1][c->j].marq = true;
                                    Mark_Case(c->i,c->j);
                                    c->i++;
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    found = true;
                                }
                            }
                        }
                        break;
                    case 4 : if (c->j > 0) {//W
                            if (!L.tab[c->i][c->j-1].marq) {
                                if (L.tab[c->i][c->j].W) {
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    L.tab[c->i][c->j-1].marq = true;
                                    Mark_Case(c->i,c->j);
                                    c->j--;
                                    empiler(*chemin,*c);
                                    taille_pile++;
                                    found = true;
                                }
                            }
                        }
                        break;

                    default:
                        cout << "valeur de porte non comprise entre 1 et 4";
                }
                // code identique aux mouvements selon les murs N/E/S/W
            }
        }
        else
        {
            *c = depiler(*chemin);
            UMark_Case(c->i, c->j);
            taille_pile--;
        }
    }

    chemin->taille = taille_pile;
    return chemin;
}

void jeu()
{
    // Lancement du jeu complet : création, affichage, déplacement, solution
    int p, q;
    bool abandon = false;
    cout << "entrez le nombre de lignes du labyrinthe" << endl;
    cin >> p;
    cout << "entrez le nombre de colonnes du labyrinthe" << endl;
    cin >> q;
    laby *L1 = My_Creation(p, q);
    InitCurses(); Show_Lab(*L1);
    mvprintw(p * 2 + 1, 0, "pour abandonner appuyez sur F1");
    deplacement(*L1, abandon);
    if (abandon)
    {
        clear();
        mvprintw(0, 0, "Vous avez abandonné : Appuyez sur une touche pour voir la solution");
        LireCommande();
        pile *sol = explorer(*L1);
        affiche_chemin(*sol, *L1);
        LireCommande();
        EndCurses();
    }
}


int main()
{
    srand (time(NULL));
    jeu();
    return 0;
}
