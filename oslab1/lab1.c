//TODO edit add int spavaj = 10; while(spavaj = sleep(spavaj) != 0);
//sve dok je broj neprospavanih kojeg assignas u spavaj razlicit od nula
//ili for(int i = 0; i < spavaj, i++) sleep(1);

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* funkcije za obradu signala, navedene ispod main-a */
void obradi_dogadjaj(int sig);

void obradi_sigterm(int sig);

void obradi_sigint(int sig);

int broj;
int nije_kraj = 1;
char status_file[] = "status.txt";
char obrada_file[] = "obrada.txt";
FILE *obrada;
FILE *status;

int main() {
    struct sigaction act;

    /* 1. maskiranje signala SIGUSR1 */
    act.sa_handler = obradi_dogadjaj; /* kojom se funkcijom signal obrađuje */
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM); /* blokirati i SIGTERM za vrijeme obrade */
    act.sa_flags = 0;                 /* naprednije mogućnosti preskočene */
    sigaction(SIGUSR1, &act, NULL);   /* maskiranje signala preko sučelja OS-a */

    /* 2. maskiranje signala SIGTERM */
    act.sa_handler = obradi_sigterm;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);

    /* 3. maskiranje signala SIGINT */
    act.sa_handler = obradi_sigint;
    sigaction(SIGINT, &act, NULL);
    printf("Program s PID = %ld krenuo s radom\n", (long) getpid());
    // TODO copy pid to clipboard on starting process

    /* Glavni posao */
    obrada = fopen(obrada_file, "r+");
    status = fopen(status_file, "r+");

    // Pročitaj broj iz status.txt
    fscanf(status, "%d", &broj);
    fclose(status);

    if (broj == 0) {
        while (fscanf(obrada, "%d", &broj) != EOF);
        {
            int pomocna = sqrt(broj);
            broj = pomocna;
        }
    }
    fclose(obrada);
    // Zapiši nula u status - trenutno računamo
    status = fopen(status_file, "w");
    fprintf(status, "0");
    fclose(status);
    obrada = fopen(obrada_file, "a");
    int x;
    while (nije_kraj) {
        broj++;
        x = broj * broj;
        fprintf(obrada, "%d\n", x);
        sleep(5);
    }
    printf("Program zavrsio s radom\n", (long) getpid());

    fclose(obrada);
    return 0;
}

void obradi_dogadjaj(int sig) { printf("Trenutni broj: %d\n", broj); }

void obradi_sigterm(int sig) {
    status = fopen(status_file, "w");
    fprintf(status, "%d", broj);
    fclose(status);
    nije_kraj = 0;
}

void obradi_sigint(int sig) {
    fclose(obrada);
    printf("\n");
    exit(1);
}
