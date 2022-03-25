/*Studenti čija je zadnja znamenka JMBAG-a neparna trebaju napisati program u kojem jedan od dva
procesa ima jednu dretvu i to ulaznu, a drugi proces dvije dretve: radnu i izlaznu.*/
/* Proces s ulaznom dretvom - svakih 1 do 5 sekundi generira nasumican broj, 1 do 100
    i upise ga u zajednicku varijablu koju dijeli s drugim procesom u kojem je radna
    dretva. Radna dretva cita taj broj, povecava ga za 1 i upisuje u drugu zajednicku
    varijablu s izlaznom dretvom. Izlazna dretva procita taj broj i zapisuje ga u 
    datoteku.
    Broj nasumicnih brojeva koje ulazna dretva generira zadaje se kao argument pri pozivanju
    glavnog programa.
    Radna dretva čeka kada je u zajedničkoj varijabli s ulaznom dretvom 0.
    Isto vrijedi za radnu/izlaznu dretvu.
    Ulazna i radna dretva komuniciraju preko zajednicke memorije koju treba stvoriti na pocetku
    programa.    
    */

int main(void) {
    int broj = 0; // nebu tak islo, trebal bu ti jedan typedef key_t int; int shmget(key_t key, int size, int flags) ;

    if (fork() == 0) {
        // sto radi proces dijete - proces s 2 dretve
    }

    return 0;
}