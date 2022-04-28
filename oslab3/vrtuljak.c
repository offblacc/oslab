#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void process_sigint(int sig) {
    exit(1);
}
void *thread_visitor(void *x) {
    printf("Thread visitor started - not functional yet\n");
}

void *thread_carousel(void *x) {
    printf("Thread carousel started - not functional yet\n");
}

int terminate_threads = 0;
int carousel_seats_num, visitors_num;
sem_t *seats_sem;

int main(int argc, char **argv) {
    // process sigint signal
    struct sigaction act;
    act.sa_handler = process_sigint;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    // ---- check if input is valid ----
    if (argc != 3) {
        printf("Two (2) arguments required, number of seats on the carousel and number of visitors (in that order)\n");
        exit(0);
    }

    carousel_seats_num = (int)strtol(argv[1], &argv[1], 10);
    visitors_num = (int)strtol(argv[2], &argv[2], 10);
    if (carousel_seats_num < 1 || visitors_num < 1) {
        printf("Invalid number of seats or visitors.\n");
        exit(0);
    }

    if ((carousel_seats_num > visitors_num)) {
        printf("Carousel has more seats than there are visitors, it will never start.\n");
        exit(0);
    }

    pthread_t thr_id[argc];  // create array of thread ids
    seats_sem = malloc(sizeof(sem_t) * carousel_seats_num);
    sem_init(seats_sem, 0, (unsigned)carousel_seats_num);  // init semaphore to number of seats on the carousel

    // create all threads
    pthread_create(&thr_id[0], NULL, thread_carousel, NULL);
    for (int i = 1; i <= visitors_num; i++) {  // thr_id[0] je rezervirano za vrtuljak, pa ide od 1 do visitors_num, ukljucivo
        pthread_create(&thr_id[i], NULL, thread_visitor, &i);
    }

    // cekaj da se sve dretve završe
    for (int i = 0; i < visitors_num + 1; i++) {
        pthread_join(thr_id[i], NULL);
    }

    return 0;
}