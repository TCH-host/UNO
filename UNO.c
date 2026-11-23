#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CARTES 108
#define MAX_MAIN 100

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

/* Enleve '\n' ou '\r' a la fin d'une chaine (version simple avec une boucle) */
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

/* Gere le tour du joueur humain */
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

/* Gere le tour de l'ordinateur */
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
    Joueur humain;
    Joueur ia;
    Carte carte_dessus;
    char couleur_active;
    int tour_du_joueur = 1; /* 1 = Humain, 0 = IA */
    int bluff_possible = 0;
    int i;

    srand((unsigned int)time(NULL));

    taille_pioche = creer_paquet(pioche);
    melanger_paquet(pioche, taille_pioche);

    printf("Entrez votre nom : ");
    if (fgets(humain.nom, sizeof(humain.nom), stdin) != NULL) {
        enlever_fin_ligne(humain.nom);
        if (humain.nom[0] == '\0') {
            strcpy(humain.nom, "Joueur Humain");
        }
    } else {
        strcpy(humain.nom, "Joueur Humain");
    }
    humain.nb_cartes = 0;
    humain.est_humain = 1;

    strcpy(ia.nom, "Ordinateur");
    ia.nb_cartes = 0;
    ia.est_humain = 0;

    for (i = 0; i < 7; i = i + 1) {
        piocher_cartes(&humain, pioche, &taille_pioche, 1);
        piocher_cartes(&ia, pioche, &taille_pioche, 1);
    }

    taille_pioche = taille_pioche - 1;
    carte_dessus = pioche[taille_pioche];
    couleur_active = carte_dessus.couleur;
    if (couleur_active == 'N') {
        couleur_active = 'R';
    }

    while (humain.nb_cartes > 0 && ia.nb_cartes > 0) {
        if (tour_du_joueur == 1) {
            Carte nouvelle = tour_humain(&humain, carte_dessus, &couleur_active, pioche, &taille_pioche, &bluff_possible);

            if (strcmp(nouvelle.valeur, carte_dessus.valeur) != 0 || nouvelle.couleur != carte_dessus.couleur) {
                carte_dessus = nouvelle;

                if (strcmp(carte_dessus.valeur, "+2") == 0) {
                    printf("L'ordinateur pioche 2 cartes et passe son tour !\n");
                    piocher_cartes(&ia, pioche, &taille_pioche, 2);
                    continue;
                } else if (strcmp(carte_dessus.valeur, "+4") == 0) {
                    int ia_appelle_bluff = (rand() % 100) < 35;
                    if (ia_appelle_bluff == 1) {
                        printf("L'ordinateur appelle au bluff sur le +4 !\n");
                        if (bluff_possible == 1) {
                            printf("Bluff decouvert ! Vous aviez la couleur precedente : vous piochez 4 cartes.\n");
                            piocher_cartes(&humain, pioche, &taille_pioche, 4);
                        } else {
                            printf("Pas de bluff ! L'ordinateur pioche 6 cartes (4 + 2 penalite) et passe son tour.\n");
                            piocher_cartes(&ia, pioche, &taille_pioche, 6);
                            continue;
                        }
                    } else {
                        printf("L'ordinateur ne conteste pas, pioche 4 cartes et passe son tour !\n");
                        piocher_cartes(&ia, pioche, &taille_pioche, 4);
                        continue;
                    }
                } else if (strcmp(carte_dessus.valeur, "PS") == 0 || strcmp(carte_dessus.valeur, "RV") == 0) {
                    printf("L'ordinateur passe son tour !\n");
                    continue;
                }
            }
            tour_du_joueur = 0;
        } else {
            Carte nouvelle = tour_ia(&ia, carte_dessus, &couleur_active, pioche, &taille_pioche);

            if (strcmp(nouvelle.valeur, carte_dessus.valeur) != 0 || nouvelle.couleur != carte_dessus.couleur) {
                carte_dessus = nouvelle;

                if (strcmp(carte_dessus.valeur, "+2") == 0) {
                    printf("Vous piochez 2 cartes et passez votre tour !\n");
                    piocher_cartes(&humain, pioche, &taille_pioche, 2);
                    continue;
                } else if (strcmp(carte_dessus.valeur, "+4") == 0) {
                    printf("Vous piochez 4 cartes et passez votre tour !\n");
                    piocher_cartes(&humain, pioche, &taille_pioche, 4);
                    continue;
                } else if (strcmp(carte_dessus.valeur, "PS") == 0 || strcmp(carte_dessus.valeur, "RV") == 0) {
                    printf("Vous passez votre tour !\n");
                    continue;
                }
            }
            tour_du_joueur = 1;
        }
    }

    printf("\n========================================\n");
    if (humain.nb_cartes == 0) {
        printf(" FELICITATIONS ! VOUS AVEZ GAGNE !\n");
    } else {
        printf(" DOMMAGE ! L'ORDINATEUR VOUS A GAGNE !\n");
    }
    printf("========================================\n");

    return 0;
}