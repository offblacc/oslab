#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int seats_num, visitors_num;
sem_t seats_sem, start_carousel_sem;

void process_sigint(int sig) {
    exit(1);
}

void *thread_visitor(void *x) {
    printf("Thread visitor started\n");
    sem_wait(&seats_sem);
    sem_post(&start_carousel_sem);
    printf("A visitor boarded the carousel.\n");
}

void *thread_carousel(void *x) {
    printf("Thread carousel started\n");
    while (true) {
        for (int i = 0; i < seats_num; i++) {
            sem_post(&seats_sem);
        }
        for (int i = 0; i < seats_num; i++) {
            sem_wait(&start_carousel_sem);
        }
        printf("Carousel is ready.\n");
        for (int i = 1; i <= 5; i++) {
            printf("Carousel is spinning (%d).\n", i);
        }
    }
}

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

    seats_num = (int)strtol(argv[1], &argv[1], 10);
    visitors_num = (int)strtol(argv[2], &argv[2], 10);
    if (seats_num < 1 || visitors_num < 1) {
        printf("Invalid number of seats or visitors.\n");
        exit(0);
    }

    if ((seats_num > visitors_num)) {
        printf("Carousel has more seats than there are visitors, it will never start.\n");
        exit(0);
    }

    pthread_t thr_id[argc];      // create array of thread ids
    sem_init(&seats_sem, 0, 0);  // init semaphore seats_sem

    sem_init(&start_carousel_sem, 0, 0);  // init semaphore start_carousel_sem

    // create all threads
    pthread_create(&thr_id[0], NULL, thread_carousel, NULL);
    for (int i = 1; i <= visitors_num; i++) {  // thr_id[0] je rezervirano za vrtuljak, pa ide od 1 do visitors_num, ukljucivo
        pthread_create(&thr_id[i], NULL, thread_visitor, NULL);
    }

    // wait for all threads to finish
    for (int i = 0; i < visitors_num + 1; i++) {
        pthread_join(thr_id[i], NULL);
    }

    return 0;
}