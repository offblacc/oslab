/*  Proces s ulaznom dretvom - svakih 1 do 5 sekundi generira nasumican broj, 1 do 100
 *  i upise ga u zajednicku varijablu koju dijeli s drugim procesom u kojem je radna
 *  dretva. Radna dretva cita taj broj, povecava ga za 1 i upisuje u drugu zajednicku
 *  varijablu s izlaznom dretvom. Izlazna dretva procita taj broj i zapisuje ga u
 *  datoteku.
 *  Broj nasumicnih brojeva koje ulazna dretva generira zadaje se kao argument pri pozivanju
 *  glavnog programa.
 *  Radna dretva čeka kada je u zajedničkoj varijabli s ulaznom dretvom 0.
 *  Isto vrijedi za radnu/izlaznu dretvu.
 *  Ulazna i radna dretva komuniciraju preko zajednicke memorije koju treba stvoriti na pocetku
 *  programa.
 *   
 * ===== exit codes and their causes =====
 * ----- code 3: shmget call returned -1, could not create memory segment -----
 */

//TODO neki sigaction postaviti?
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int seg_id;
int *shared_variable;

void thread_in() {
    int sleep_for = rand() % 5 + 1;
    for (int i = 0; i < sleep_for; i++) {
        sleep(1);
    }  // TODO ovo se salje preko zajednicke memorije, shm funkcije
    *shared_variable = rand() % 100 + 1;
}

int main(void) {
    srand(time(NULL));
    
    seg_id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    if (seg_id == -1) exit(3);
    shared_variable = (int *) shmat(seg_id, NULL, 0);
    
    thread_in();

    if (fork() == 0) { // sad smo u drugom procesu, drugoj dretvi (radnoj - ona obraduje broj)
        int pvar = *shared_variable;
        pvar++;
        
        exit(0);
    }

    // int shmget(key_t key, size_t size, int flags);

    // UlaznaDretva operacije

    exit(0);
    if (fork == 0) {
    }

    return 0;
}