#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void process_sigint(int sig);
void *programer(void *x);
void enter(int type);
void mutex_enter(int type);
void mutex_exit(int type);

pthread_mutex_t mutex;
pthread_cond_t cond[2];
int eating[] = {0, 0};
int in_line[] = {0, 0};
int have_eaten[] = {0, 0};
int starving_limit = 2;
int chairs;

int main() {
    struct sigaction act;
    act.sa_handler = process_sigint;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    printf("Enter a number of places in the restaurant: ");
    scanf("%d", &chairs);
    printf("There are %d places in the restaurant.\n", chairs);
    printf("Enter a number of MS programmers: ");
    int ms_num;
    scanf("%d", &ms_num);
    printf("Enter a number of Linux programmers: ");
    int linux_num;
    scanf("%d", &linux_num);

    pthread_mutex_init(&mutex, NULL);

    pthread_t thr_id[ms_num + linux_num];
    int zero = 0;
    int one = 1;
    int j = 0;

    for (; j < ms_num; j++) {
        if (pthread_create(&thr_id[j], NULL, programer, &zero) != 0) {
            printf("Could not create thread!\n");
            exit(1);
        }
    }
    if (pthread_create(&thr_id[j], NULL, programer, &one) != 0) {
            printf("Could not create thread!\n");
            exit(1);
        }

    for (int i = 0; i < ms_num + linux_num; i++) {
        pthread_join(thr_id[i], NULL);
    }

    return 0;
}

// TODO uvjet, restoran ne smije biti pun
void process_sigint(int sig) {
    printf("Received SIGINT.\n");
    printf("Currently in line: %d, %d.\n", in_line[0], in_line[1]);
    printf("Currently eating: %d, %d.\n", eating[0], eating[1]);
    printf("Currently have eaten: %d, %d.\n", have_eaten[0], have_eaten[1]);
    exit(1);
}

void mutex_enter(int type) {
    pthread_mutex_lock(&mutex);
    in_line[type]++;
    while ((eating[1 - type] > 0) || (have_eaten[type] > starving_limit && in_line[1 - type] > 0) || eating[0] + eating[1] >= chairs) {
        pthread_cond_wait(&cond[type], &mutex);
    }
    eating[type]++;
    in_line[type]--;
    have_eaten[1 - type] = 0;
    pthread_mutex_unlock(&mutex);
}

void mutex_exit(int type) {
    pthread_mutex_lock(&mutex);
    eating[type]--;
    have_eaten[type]++;
    if (eating[type] == 0) pthread_cond_broadcast(&cond[1 - type]);
    pthread_mutex_unlock(&mutex);
}

void *programer(void *x) {
    int type = *((int *)x);
    if (type != 0 && type != 1) {
        printf("Invalid programmer type!\n");
        return NULL;
    }

    mutex_enter(type);
    printf("A %s programmer is eating!\n", type == 0 ? "MS" : "Linux");
    for (int i = 0; i < 2; i++) {
        sleep(1);
    }
    printf("A %s programmer is exiting the restaurant!\n", type == 0 ? "MS" : "Linux");
    mutex_exit(type);
}