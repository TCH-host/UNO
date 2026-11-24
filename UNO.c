#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CARTES 108
#define MAX_MAIN 100
#define MAX_JOUEURS 6

/* Structure d'une carte */
typedef struct {
    char valeur[5]; /* "0"-"9", "+2", "PS", "RV", "+4", "JJ" */
    char couleur;    /* 'R', 'V', 'J', 'B', 'N' */
} Carte;

/* Structure d'un joueur */
typedef struct {
    char nom[50];
    Carte main[MAX_MAIN];
    int nb_cartes;
    int est_humain;
} Joueur;

/* Vide ce qui reste sur la ligne du clavier */
void vider_buffer(void) {
    int c;
    c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

/* Enleve '\n' ou '\r' a la fin d'une chaine */
void enlever_fin_ligne(char texte[]) {
    int i = 0;
    while (texte[i] != '\0') {
        if (texte[i] == '\n' || texte[i] == '\r') {
            texte[i] = '\0';
            break;
        }
        i = i + 1;
    }
}

/* Calcule l'indice du joueur suivant selon le sens (1 = horaire, -1 = anti-horaire) */
int prochain_joueur(int tour, int sens, int nb_joueurs) {
    return (tour + sens + nb_joueurs) % nb_joueurs;
}

/* Verifie si aucun joueur n'a encore vide sa main */
int partie_en_cours(const Joueur joueurs[], int nb_joueurs) {
    int i;
    for (i = 0; i < nb_joueurs; i = i + 1) {
        if (joueurs[i].nb_cartes <= 0) {
            return 0;
        }
    }
    return 1;
}

/* Affiche une carte en ASCII */
void afficher_carte_ascii(Carte c) {
    printf("+-------+\n");
    printf("| %-2s  %c |\n", c.valeur, c.couleur);
    printf("|       |\n");
    printf("| %c  %-2s |\n", c.couleur, c.valeur);
    printf("+-------+\n");
}

/* Cree les 108 cartes du UNO */
int creer_paquet(Carte paquet[]) {
    int idx = 0;
    char couleurs[4] = {'R', 'V', 'J', 'B'};
    char *speciales[11] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "+2", "PS"};
    int c, i, copie;

    for (c = 0; c < 4; c = c + 1) {
        char col = couleurs[c];

        /* Un seul "0" par couleur */
        strcpy(paquet[idx].valeur, "0");
        paquet[idx].couleur = col;
        idx = idx + 1;

        /* Deux exemplaires de "1" a "9", "+2" et "PS" */
        for (i = 0; i < 11; i = i + 1) {
            for (copie = 0; copie < 2; copie = copie + 1) {
                strcpy(paquet[idx].valeur, speciales[i]);
                paquet[idx].couleur = col;
                idx = idx + 1;
            }
        }

        /* Deux cartes "RV" par couleur */
        for (copie = 0; copie < 2; copie = copie + 1) {
            strcpy(paquet[idx].valeur, "RV");
            paquet[idx].couleur = col;
            idx = idx + 1;
        }
    }

    /* 4 Jokers ("JJ") et 4 "+4" noirs ('N') */
    for (i = 0; i < 4; i = i + 1) {
        strcpy(paquet[idx].valeur, "JJ");
        paquet[idx].couleur = 'N';
        idx = idx + 1;

        strcpy(paquet[idx].valeur, "+4");
        paquet[idx].couleur = 'N';
        idx = idx + 1;
    }

    return idx;
}

/* Melange le paquet */
void melanger_paquet(Carte paquet[], int taille) {
    int i, j;
    Carte temp;

    for (i = taille - 1; i > 0; i = i - 1) {
        j = rand() % (i + 1);
        temp = paquet[i];
        paquet[i] = paquet[j];
        paquet[j] = temp;
    }
}

/* Verifie si une carte peut etre jouee */
int est_coup_valide(Carte carte, Carte carte_dessus, char couleur_active) {
    if (carte.couleur == 'N') {
        return 1;
    }
    if (carte.couleur == couleur_active) {
        return 1;
    }
    if (strcmp(carte.valeur, carte_dessus.valeur) == 0) {
        return 1;
    }
    return 0;
}

/* Verifie si un joueur a au moins une carte d'une couleur donnee */
int possede_couleur(const Joueur *joueur, char couleur) {
    int i;
    for (i = 0; i < joueur->nb_cartes; i = i + 1) {
        if (joueur->main[i].couleur == couleur) {
            return 1;
        }
    }
    return 0;
}

/* Fait piocher nb cartes a un joueur */
void piocher_cartes(Joueur *joueur, Carte pioche[], int *taille_pioche, int nb) {
    int i;
    for (i = 0; i < nb; i = i + 1) {
        if (*taille_pioche > 0 && joueur->nb_cartes < MAX_MAIN) {
            *taille_pioche = *taille_pioche - 1;
            joueur->main[joueur->nb_cartes] = pioche[*taille_pioche];
            joueur->nb_cartes = joueur->nb_cartes + 1;
        }
    }
}

/* Retire la carte a la position idx de la main d'un joueur */
void retirer_carte_main(Joueur *joueur, int idx) {
    int i;
    for (i = idx; i < joueur->nb_cartes - 1; i = i + 1) {
        joueur->main[i] = joueur->main[i + 1];
    }
    joueur->nb_cartes = joueur->nb_cartes - 1;
}

/* Demande au joueur humain de choisir une couleur */
char choisir_couleur_humain(void) {
    char col[10];
    int ok;

    while (1) {
        printf("Choisissez une couleur (R, V, J, B) : ");
        ok = scanf("%9s", col);
        if (ok != 1) {
            vider_buffer();
        } else {
            vider_buffer();
            if (col[0] == 'r' || col[0] == 'R') {
                return 'R';
            }
            if (col[0] == 'v' || col[0] == 'V') {
                return 'V';
            }
            if (col[0] == 'j' || col[0] == 'J') {
                return 'J';
            }
            if (col[0] == 'b' || col[0] == 'B') {
                return 'B';
            }
            printf("Couleur invalide.\n");
        }
    }
}

/* Gere l'annonce UNO et les penalites pour le joueur humain */
void verifier_regle_uno_humain(Joueur *joueur, Carte pioche[], int *taille_pioche) {
    char saisie[32];
    int a_dit_uno;

    printf("Appuyez sur Entree pour continuer (ou saisissez UNO si applicable) : ");
    if (fgets(saisie, sizeof(saisie), stdin) == NULL) {
        saisie[0] = '\0';
    }
    enlever_fin_ligne(saisie);

    if (strcmp(saisie, "UNO") == 0 || strcmp(saisie, "uno") == 0) {
        a_dit_uno = 1;
    } else {
        a_dit_uno = 0;
    }

    if (joueur->nb_cartes == 1) {
        if (a_dit_uno == 1) {
            printf("Bien joue, vous avez annonce UNO !\n");
        } else {
            printf("Vous avez oublie de crier UNO ! Penalite : vous piochez 2 cartes.\n");
            piocher_cartes(joueur, pioche, taille_pioche, 2);
        }
    } else if (joueur->nb_cartes > 1 && a_dit_uno == 1) {
        printf("Vous avez saisi UNO alors qu'il vous reste %d cartes ! Penalite : 2 cartes.\n", joueur->nb_cartes);
        piocher_cartes(joueur, pioche, taille_pioche, 2);
    }
}

/* Gere le tour d'un joueur humain */
Carte tour_humain(Joueur *joueur, Carte carte_dessus, char *couleur_active, Carte pioche[], int *taille_pioche, int *bluff_possible) {
    int i;
    char choix[16];
    int ok;

    *bluff_possible = 0;

    printf("\n========================================\n");
    printf(" C'est au tour de : %s (%d cartes)\n", joueur->nom, joueur->nb_cartes);
    printf("========================================\n");
    printf(" SOMMET DE LA DEFAUSSE :\n");
    afficher_carte_ascii(carte_dessus);
    printf(" Couleur active : %c\n", *couleur_active);

    printf("\n Votre main :\n");
    for (i = 0; i < joueur->nb_cartes; i = i + 1) {
        printf("\n[%d]\n", i + 1);
        afficher_carte_ascii(joueur->main[i]);
    }

    printf("\nChoisissez le numero de carte (1 a %d) ou tapez 'p' pour piocher : ", joueur->nb_cartes);
    ok = scanf("%15s", choix);
    if (ok != 1) {
        vider_buffer();
        return carte_dessus;
    }
    vider_buffer();

    /* Cas ou le joueur choisit de piocher */
    if (strcmp(choix, "p") == 0 || strcmp(choix, "P") == 0) {
        if (*taille_pioche > 0 && joueur->nb_cartes < MAX_MAIN) {
            Carte carte_piochee;
            piocher_cartes(joueur, pioche, taille_pioche, 1);
            carte_piochee = joueur->main[joueur->nb_cartes - 1];

            printf("Vous avez pioche :\n");
            afficher_carte_ascii(carte_piochee);

            if (est_coup_valide(carte_piochee, carte_dessus, *couleur_active) == 1) {
                char rep[10];
                printf("Voulez-vous jouer cette carte tout de suite ? (o/n) : ");
                if (scanf("%9s", rep) == 1) {
                    vider_buffer();
                    if (rep[0] == 'o' || rep[0] == 'O') {
                        int idx_jouee = joueur->nb_cartes - 1;
                        int avait_couleur_active = possede_couleur(joueur, *couleur_active);

                        retirer_carte_main(joueur, idx_jouee);

                        if (carte_piochee.couleur == 'N') {
                            *couleur_active = choisir_couleur_humain();
                            if (strcmp(carte_piochee.valeur, "+4") == 0) {
                                *bluff_possible = avait_couleur_active;
                            }
                        } else {
                            *couleur_active = carte_piochee.couleur;
                        }

                        verifier_regle_uno_humain(joueur, pioche, taille_pioche);
                        return carte_piochee;
                    }
                }
            }
        } else {
            printf("La pioche est vide !\n");
        }
        return carte_dessus;
    }

    /* Cas ou le joueur choisit une carte par son numero */
    {
        int idx = atoi(choix) - 1;
        Carte carte_choisie;
        int avait_couleur_active;

        if (idx < 0 || idx >= joueur->nb_cartes) {
            printf("Choix invalide, tour passe.\n");
            return carte_dessus;
        }

        carte_choisie = joueur->main[idx];
        if (est_coup_valide(carte_choisie, carte_dessus, *couleur_active) == 0) {
            printf("Coup invalide avec cette carte, tour passe.\n");
            return carte_dessus;
        }

        avait_couleur_active = possede_couleur(joueur, *couleur_active);
        retirer_carte_main(joueur, idx);

        if (carte_choisie.couleur == 'N') {
            *couleur_active = choisir_couleur_humain();
            if (strcmp(carte_choisie.valeur, "+4") == 0) {
                *bluff_possible = avait_couleur_active;
            }
        } else {
            *couleur_active = carte_choisie.couleur;
        }

        verifier_regle_uno_humain(joueur, pioche, taille_pioche);
        return carte_choisie;
    }
}

/* Gere le tour d'un joueur ordinateur */
Carte tour_ia(Joueur *ia, Carte carte_dessus, char *couleur_active, Carte pioche[], int *taille_pioche) {
    int i;
    char couleurs[4] = {'R', 'V', 'J', 'B'};

    printf("\n========================================\n");
    printf(" C'est au tour de : %s (%d cartes)\n", ia->nom, ia->nb_cartes);
    printf("========================================\n");

    for (i = 0; i < ia->nb_cartes; i = i + 1) {
        if (est_coup_valide(ia->main[i], carte_dessus, *couleur_active) == 1) {
            Carte carte_jouee = ia->main[i];
            retirer_carte_main(ia, i);

            printf("%s joue :\n", ia->nom);
            afficher_carte_ascii(carte_jouee);

            if (carte_jouee.couleur == 'N') {
                *couleur_active = couleurs[rand() % 4];
                printf("%s choisit la couleur : %c\n", ia->nom, *couleur_active);
            } else {
                *couleur_active = carte_jouee.couleur;
            }

            if (ia->nb_cartes == 1) {
                printf("%s crie : UNO !\n", ia->nom);
            }

            return carte_jouee;
        }
    }

    if (*taille_pioche > 0 && ia->nb_cartes < MAX_MAIN) {
        Carte carte_piochee;
        piocher_cartes(ia, pioche, taille_pioche, 1);
        carte_piochee = ia->main[ia->nb_cartes - 1];
        printf("%s n'avait pas de carte jouable et a pioche.\n", ia->nom);

        if (est_coup_valide(carte_piochee, carte_dessus, *couleur_active) == 1) {
            retirer_carte_main(ia, ia->nb_cartes - 1);
            printf("%s joue la carte piochee :\n", ia->nom);
            afficher_carte_ascii(carte_piochee);

            if (carte_piochee.couleur == 'N') {
                *couleur_active = couleurs[rand() % 4];
                printf("%s choisit la couleur : %c\n", ia->nom, *couleur_active);
            } else {
                *couleur_active = carte_piochee.couleur;
            }

            if (ia->nb_cartes == 1) {
                printf("%s crie : UNO !\n", ia->nom);
            }

            return carte_piochee;
        }
    } else {
        printf("La pioche est vide !\n");
    }

    return carte_dessus;
}

int main(void) {
    Carte pioche[MAX_CARTES];
    int taille_pioche;
    Joueur joueurs[MAX_JOUEURS];
    int nb_joueurs = 0;
    Carte carte_dessus;
    char couleur_active;
    int tour = 0;
    int sens = 1; /* 1 = sens normal, -1 = sens inverse */
    int bluff_possible = 0;
    int i, j;

    srand((unsigned int)time(NULL));

    /* Choix du nombre de joueurs */
    while (nb_joueurs < 2 || nb_joueurs > MAX_JOUEURS) {
        char buf[16];
        printf("Nombre de joueurs (2 a %d) : ", MAX_JOUEURS);
        if (scanf("%15s", buf) == 1) {
            vider_buffer();
            nb_joueurs = atoi(buf);
            if (nb_joueurs < 2 || nb_joueurs > MAX_JOUEURS) {
                printf("Veuillez saisir un nombre entre 2 et %d.\n", MAX_JOUEURS);
            }
        } else {
            vider_buffer();
        }
    }

    /* Configuration de chaque joueur */
    for (j = 0; j < nb_joueurs; j = j + 1) {
        char type_buf[16];
        int type_val = -1;

        printf("\n--- Joueur %d ---\n", j + 1);
        printf("Nom du joueur %d : ", j + 1);
        if (fgets(joueurs[j].nom, sizeof(joueurs[j].nom), stdin) != NULL) {
            enlever_fin_ligne(joueurs[j].nom);
        } else {
            joueurs[j].nom[0] = '\0';
        }

        while (type_val != 0 && type_val != 1) {
            printf("Type (1 = Humain, 0 = Ordinateur) : ");
            if (scanf("%15s", type_buf) == 1) {
                vider_buffer();
                if (strcmp(type_buf, "1") == 0) {
                    type_val = 1;
                } else if (strcmp(type_buf, "0") == 0) {
                    type_val = 0;
                } else {
                    printf("Reponse invalide, tapez 1 ou 0.\n");
                }
            } else {
                vider_buffer();
            }
        }

        joueurs[j].est_humain = type_val;
        joueurs[j].nb_cartes = 0;

        if (joueurs[j].nom[0] == '\0') {
            if (joueurs[j].est_humain == 1) {
                sprintf(joueurs[j].nom, "Humain %d", j + 1);
            } else {
                sprintf(joueurs[j].nom, "Ordinateur %d", j + 1);
            }
        }
    }

    taille_pioche = creer_paquet(pioche);
    melanger_paquet(pioche, taille_pioche);

    /* Distribution de 7 cartes a chaque joueur */
    for (i = 0; i < 7; i = i + 1) {
        for (j = 0; j < nb_joueurs; j = j + 1) {
            piocher_cartes(&joueurs[j], pioche, &taille_pioche, 1);
        }
    }

    /* Premiere carte de la defausse */
    taille_pioche = taille_pioche - 1;
    carte_dessus = pioche[taille_pioche];
    couleur_active = carte_dessus.couleur;
    if (couleur_active == 'N') {
        couleur_active = 'R';
    }

    while (partie_en_cours(joueurs, nb_joueurs) == 1) {
        Joueur *courant = &joueurs[tour];
        Carte nouvelle;

        if (courant->est_humain == 1) {
            nouvelle = tour_humain(courant, carte_dessus, &couleur_active, pioche, &taille_pioche, &bluff_possible);
        } else {
            bluff_possible = 0;
            nouvelle = tour_ia(courant, carte_dessus, &couleur_active, pioche, &taille_pioche);
        }

        if (strcmp(nouvelle.valeur, carte_dessus.valeur) != 0 || nouvelle.couleur != carte_dessus.couleur) {
            int suivant = prochain_joueur(tour, sens, nb_joueurs);
            carte_dessus = nouvelle;

            if (strcmp(carte_dessus.valeur, "RV") == 0) {
                sens = -sens;
                printf("Le sens du jeu est inverse !\n");
                if (nb_joueurs == 2) {
                    /* A 2 joueurs, RV fait rejouer le joueur courant */
                    continue;
                }
            } else if (strcmp(carte_dessus.valeur, "+2") == 0) {
                printf("%s pioche 2 cartes et passe son tour !\n", joueurs[suivant].nom);
                piocher_cartes(&joueurs[suivant], pioche, &taille_pioche, 2);
                tour = prochain_joueur(suivant, sens, nb_joueurs);
                continue;
            } else if (strcmp(carte_dessus.valeur, "+4") == 0) {
                if (courant->est_humain == 1 && joueurs[suivant].est_humain == 0 && ((rand() % 100) < 35)) {
                    printf("%s appelle au bluff sur le +4 !\n", joueurs[suivant].nom);
                    if (bluff_possible == 1) {
                        printf("Bluff decouvert ! %s avait la couleur precedente : il pioche 4 cartes.\n", courant->nom);
                        piocher_cartes(courant, pioche, &taille_pioche, 4);
                    } else {
                        printf("Pas de bluff ! %s pioche 6 cartes (4 + 2 penalite) et passe son tour.\n", joueurs[suivant].nom);
                        piocher_cartes(&joueurs[suivant], pioche, &taille_pioche, 6);
                        tour = prochain_joueur(suivant, sens, nb_joueurs);
                        continue;
                    }
                } else {
                    printf("%s pioche 4 cartes et passe son tour !\n", joueurs[suivant].nom);
                    piocher_cartes(&joueurs[suivant], pioche, &taille_pioche, 4);
                    tour = prochain_joueur(suivant, sens, nb_joueurs);
                    continue;
                }
            } else if (strcmp(carte_dessus.valeur, "PS") == 0) {
                printf("%s passe son tour !\n", joueurs[suivant].nom);
                tour = prochain_joueur(suivant, sens, nb_joueurs);
                continue;
            }
        }

        tour = prochain_joueur(tour, sens, nb_joueurs);
    }

    printf("\n========================================\n");
    for (j = 0; j < nb_joueurs; j = j + 1) {
        if (joueurs[j].nb_cartes == 0) {
            printf(" FELICITATIONS ! %s A GAGNE !\n", joueurs[j].nom);
            break;
        }
    }
    printf("========================================\n");

    return 0;
}