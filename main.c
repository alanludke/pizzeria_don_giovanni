#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include "pizzeria.h"
#include "helper.h"

sem_t sem_;

void *thread_pizzaiolo(void *args) {
    for (int i = 0; i < *(int*)args; i++) {
        sem_wait(&sem_a);
        fprintf(out, "A");
        qtdA++;
        fflush(stdout);
        sem_post(&sem_a);
        sem_wait(&sem_b);
        sem_post(&sem_a);

    // Importante para que vocês vejam o progresso do programa
    // mesmo que o programa trave em um sem_wait().
    }
    return NULL;
}


int main(int argc, char** argv) {
    int tam_forno = 1, n_pizzaiolos = 1, n_mesas = 10, n_garcons = 1,
        tam_deck = 0, n_grupos = 0, segs_execucao = 10;
    
    if (argc < 7) {
        printf("Faltaram argumentos!\n"
               "Uso:%s tam_forno n_pizzaiolos n_mesas n_garcons "
               "tam_deck n_grupos [segs_execucao]\n", argv[0]);
        return 1;
    }
    tam_forno    = atoi(argv[1]);
    n_pizzaiolos = atoi(argv[2]);
    n_mesas      = atoi(argv[3]);
    n_garcons    = atoi(argv[4]);
    tam_deck     = atoi(argv[5]);
    n_grupos     = atoi(argv[6]);
    if (argc > 7) 
        segs_execucao = atoi(argv[7]);

   
    helper_init(tam_forno, n_pizzaiolos, n_mesas, n_garcons, tam_deck, n_grupos); // está contido em helper.c(não mexeremos)

    //declarando as threads
    pthread_t pizzaiolos[n_pizzaiolos];
    pthread_t garcons[n_garcons];

    //loop para criar as threads de pizzaiolos
    for (int i = 0; i < n_pizzaiolos; i++) {
        pthread_create(&pizzaiolos[i], NULL, thread_pizzaiolo, &segs_execucao);
    }

    //loop para criar as threads de garçons
    for (int i = 0; i < n_garcons; i++) {
        pthread_create(&garcons[i], NULL, thread_garcom, , &segs_execucao);
    }

    pizzeria_init(tam_forno, n_pizzaiolos, n_mesas, n_garcons, tam_deck, n_grupos);
    pizzeria_open();

    printf("Executando simulação por %d segundos\n", segs_execucao);
    sleep(segs_execucao);
    printf("Passados %d segundos, fechando pizzaria\n", segs_execucao);
    
    
    pizzeria_close();
    pizzeria_destroy();
    helper_destroy();

    //TODO: report de pizzas queimadas, clientes atendidos, etc.

    return 0;
}
