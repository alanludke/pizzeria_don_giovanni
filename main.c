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
    pizzeria_init(tam_forno, n_pizzaiolos, n_mesas, n_garcons, tam_deck, n_grupos);
    pizzeria_open();

    printf("Executando simulação por %d segundos\n", segs_execucao);
    sleep(segs_execucao);
    printf("Passados %d segundos, fechando pizzaria\n", segs_execucao);

    pizzeria_close();
    pizzeria_destroy();
    helper_destroy();

    printf("Passados %d segundos, fechando pizzaria\n\n", segs_execucao);

    //printf("Relatório do dia:\nNúmero de pizzas queimadas: %d/%d\nClientes atendidos: %d\nassdadssdasdasda: %d\n", g_hlp_pizzas_queimadas, g_n_pizzas, -999, -999);

    return 0;
}
