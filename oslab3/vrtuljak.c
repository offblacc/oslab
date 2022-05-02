#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int parent_pid;
int seats_num, visitors_num;
sem_t *seats_free, *seats_taken, *allowed_out, *out;

void process_sigint(int sig) {
    int free;
    sem_getvalue(seats_free, &free);
    if (getpid() == parent_pid) {
        printf("\n\nPARENT PROCESS: SIGINT received, currently boarded visitors: %d.\n", seats_num - free);
        printf("Exiting all child processes and parent process...\n");
    }
    exit(0);
}

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = process_sigint;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

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

    int seg_id[4];

    seg_id[0] = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    if (seg_id[0] == -1) exit(1);
    seats_free = (sem_t *)shmat(seg_id[0], NULL, 0);

    seg_id[1] = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    if (seg_id[1] == -1) exit(1);
    seats_taken = (sem_t *)shmat(seg_id[1], NULL, 0);

    seg_id[2] = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    if (seg_id[2] == -1) exit(1);
    allowed_out = (sem_t *)shmat(seg_id[2], NULL, 0);

    seg_id[3] = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    if (seg_id[3] == -1) exit(1);
    out = (sem_t *)shmat(seg_id[3], NULL, 0);

    sem_init(seats_free, 1, 0);
    sem_init(seats_taken, 1, 0);
    sem_init(allowed_out, 1, 0);
    sem_init(out, 1, 0);

    // ------------- child processes, visitors -------------
    for (int i = 0; i < visitors_num; i++) {
        if (fork() == 0) {
            sem_wait(seats_free);
            sem_post(seats_taken);
            printf("Visitor %d is now on the carousel.\n", i + 1);
            sem_wait(allowed_out);
            sem_post(out);
            printf("Visitor %d is leaving the carousel.\n", i + 1);
            exit(0);
        }
    }

    // ------------- parent process, carousel --------------
    parent_pid = getpid();
    while (true) {
        for (int i = 0; i < seats_num; i++) {
            sem_post(seats_free);
        }
        for (int i = 0; i < seats_num; i++) {
            sem_wait(seats_taken);
        }
        sleep(1);
        printf("Carousel is ready.\n");
        for (int i = 1; i <= 5; i++) {
            sleep(1);
            printf("Carousel is spinning (%d).\n", i);
        }
        sleep(1);
        printf("Carousel is stopped, visitors are now leaving the carousel.\n");
        for (int i = 0; i < seats_num; i++) {
            sem_post(allowed_out);
        }
        for (int i = 0; i < seats_num; i++) {
            sem_wait(out);
        }
    }

    // ------------- cleanup -------------
    for (int i = 0; i < visitors_num; i++) {
        (void)wait(NULL);
    }
    (void)shmdt((char *)seats_free);
    (void)shmdt((char *)seats_taken);
    (void)shmdt((char *)allowed_out);
    (void)shmdt((char *)out);
    for (int i = 0; i < visitors_num; i++) {
        (void)shmctl(seg_id[i], IPC_RMID, NULL);
    }
}