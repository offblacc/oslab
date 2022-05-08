#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void process_sigint();
void *programmer(void *x);
void mutex_enter(int type);
void mutex_exit(int type);

pthread_mutex_t mutex;
pthread_cond_t cond[2];
int eating[] = {0, 0};
int in_line[] = {0, 0};
int have_eaten[] = {0, 0};
int starving_limit = 2;
int eating_duration = 0;  // seconds
int meals_num;
int chairs;

int main() {
    struct sigaction act;
    act.sa_handler = process_sigint;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    printf("Starvation limit set to %d, eating duration set to %d seconds.\n", starving_limit, eating_duration);
    printf("Enter a number of places in the restaurant (zero for infinite): ");
    scanf("%d", &chairs);
    chairs = (chairs == 0) ? INT_MAX : chairs;
    printf("There are ");
    if (chairs == INT_MAX)
        printf("infinite ");
    else
        printf("%d ", chairs);
    printf("places in the restaurant\n");

    // to avoid an infinite while loop, a for loop will suffice for just having this 'get back in line' logic, for various settings (number of seats, number of meals)
    printf("Enter a number of times each programmer has to eat: ");
    scanf("%d", &meals_num);

    int ms_num, linux_num;
    printf("Enter a number of MS programmers: ");
    scanf("%d", &ms_num);
    printf("Enter a number of Linux programmers: ");
    scanf("%d", &linux_num);
    printf(" ------- starting simulation ------- \n");

    pthread_mutex_init(&mutex, NULL);

    pthread_t thr_id[ms_num + linux_num];
    int j = 0;

    for (; j < ms_num; j++) {
        if (pthread_create(&thr_id[j], NULL, programmer, &(int){0}) != 0) {
            printf("Could not create thread!\n");
            exit(1);
        }
    }

    for (; j < ms_num + linux_num; j++) {
        if (pthread_create(&thr_id[j], NULL, programmer, &(int){1}) != 0) {
            printf("Could not create thread!\n");
            exit(1);
        }
    }

    for (int i = 0; i < ms_num + linux_num; i++) {
        pthread_join(thr_id[i], NULL);
    }

    printf(" --------------- end --------------- \n");
    return 0;
}

void process_sigint() {
    printf("\nReceived SIGINT.\n");
    printf("Currently in line: %d, %d.\n", in_line[0], in_line[1]);
    printf("Currently eating: %d, %d.\n", eating[0], eating[1]);
    printf("Currently have eaten: %d, %d.\n", have_eaten[0], have_eaten[1]);
    exit(1);
}

void *programmer(void *x) {
    int type = *((int *)x);
    if (type != 0 && type != 1) {
        printf("Invalid programmer type!\n");
        return NULL;
    }
    // eat meals_num times
    for (int i = 0; i < meals_num; i++) {
        mutex_enter(type);
        printf("A %s programmer is eating! (meal no. %d)\n", type == 0 ? "MS" : "Linux", i + 1);
        for (int i = 0; i < eating_duration; i++) {
            sleep(1);
        }
        printf("A %s programmer is exiting the restaurant!\n", type == 0 ? "MS" : "Linux");
        mutex_exit(type);
    }
}

void mutex_enter(int type) {
    pthread_mutex_lock(&mutex);
    in_line[type]++;
    while ((eating[1 - type] > 0) || (have_eaten[type] > starving_limit && in_line[1 - type] > 0) || eating[0] + eating[1] >= chairs) // but cannot be >=, only <=
        pthread_cond_wait(&cond[type], &mutex);

    eating[type]++;
    in_line[type]--;
    have_eaten[1 - type] = 0;
    pthread_mutex_unlock(&mutex);
}

void mutex_exit(int type) {
    pthread_mutex_lock(&mutex);
    eating[type]--;
    have_eaten[type]++;

    if (eating[type] == 0) {                      // if the restaurant is empty (no need to check count of other type - won't happen due to synchronization contitions, only one type can be eating at a time)
        if (in_line[1 - type] == 0) {             // if the other type is not in line waiting
            pthread_cond_broadcast(&cond[type]);  // cond broadcast to this type
        } else {
            pthread_cond_broadcast(&cond[1 - type]);  // else cond broadcast to the other type and let them enter the restaurant
        }
    }
    pthread_mutex_unlock(&mutex);
}
