#ifndef __PIZZERIA_H_
#define __PIZZERIA_H_

#include <semaphore.h>
#include <pthread.h>


// Semáforos
sem_t sem_pizzaiolos;
sem_t sem_garcons;
sem_t s_balcao_cozinheiros;

// Mutexes para indices
pthread_mutex_t mut_entrega_pizzaiolo;
pthread_mutex_t mut_coleta_garcom;

// Indices dos cozinheiros a colocar pratos
// e garcons a pegar
int index_cozinheiro, index_garcon;

// buffer de pizzas a serem entregues
prato_t** pratos_prontos;

// Threads pizzaiolos
pthread_t* threads_pizzaiolos;

int* buffer_pedidos;

typedef struct cliente_s cliente_t;

typedef struct pedido_s {
    cliente_t* cliente; ///< IMPORTANTE! NÃO REMOVER
    int id;             ///< IMPORTANTE! NÃO REMOVER
    int sabor;          ///< IMPORTANTE! NÃO REMOVER

    /* você pode adicionar coisas aqui */
} pedido_t;

typedef struct pizza_s {
    int sabor;          ///< IMPORTANTE! NÃO REMOVER
    int fatias;         ///< IMPORTANTE! NÃO REMOVER
    pedido_t* pedido;   ///< IMPORTANTE! NÃO REMOVER
    struct timespec ts; ///< IMPORTANTE! NÃO REMOVER

    /* você pode adicionar coisas aqui */
} pizza_t;

void pizzeria_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                   int n_garcons, int tam_deck, int n_grupos);

void pizzeria_close();
void pizzeria_destroy();

void pizza_assada(pizza_t* pizza);
int  pegar_mesas(int tam_grupo);

void garcom_tchau(int tam_grupo);
void garcom_chamar();
void fazer_pedido(pedido_t* pedido);

int pizza_pegar_fatia(pizza_t* pizza);

#endif /*__PIZZERIA_H_*/
