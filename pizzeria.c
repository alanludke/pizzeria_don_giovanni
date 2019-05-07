#include "pizzeria.h"
#include "queue.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// Semaforos
sem_t sem_forno, sem_pizzaiolos, sem_mesas, sem_garcons, sem_tam_deck;

// Mutexes
pthread_mutex_t mutex_pa;

queue_t queue_pedidos, queue_balcao;

void *thread_pizzaiolo(void* arg) {
	// retira da queue e cria pedido a partir deste
	pedido_t* pedido = (pedido_t*) queue_wait(&queue_pedidos);

	pizza_t* pizza = pizzaiolo_montar_pizza(pedido);

	pizza->assada = 0;
	// ocupa forno até pizza ser assada
	sem_wait(&sem_forno);
	// utiliza a pá
	pthread_mutex_lock(&mutex_pa);
	pizzaiolo_colocar_forno(pizza);

	// desocupa a pá
	pthread_mutex_unlock(&mutex_pa);

	// espera enquanto pizza não está assada
	while(!pizza->assada);
	// utiliza a pá
	pthread_mutex_lock(&mutex_pa);

	pizzaiolo_retirar_forno(pizza);

	// desocupa a pá
	pthread_mutex_unlock(&mutex_pa);

	// desocupa forno
	sem_post(&sem_forno);

	// espera enquanto não há local vazio no balcao ***pode nao precisar pois ja se espera
	//o balcao ser desocupado atraves de um semaforo dentro da funcao queue_push_back
	//while (!queue_empty(&queue_balcao));	***

	//seta mtx_pegador para destravado
	pthread_mutex_init(&pizza->mtx_pegador, NULL);

	// se possível coloca a pizza e pegador no balcao
	queue_push_back(&queue_balcao, pizza);

	// Desalocando pedido
	free(pedido);

	// Post para ter cozinheiro livre
	sem_post(&sem_pizzaiolos);

  pthread_exit(NULL);
}


/*
Inicializa quaisquer recursos e estruturas de dados que sejam necessários antes da pizzeria poder receber clientes.
Chamada pela função main() antes de qualquer outra função.
*/
void pizzeria_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                   int n_garcons, int tam_deck, int n_grupos) {

	queue_init(&queue_pedidos, tam_deck);
  queue_init(&queue_balcao, 1);
	sem_init(&sem_forno, 0, tam_forno);
	sem_init(&sem_pizzaiolos, 0, n_pizzaiolos);
	sem_init(&sem_mesas, 0, n_mesas);
	sem_init(&sem_garcons, 0, n_garcons);
	sem_init(&sem_tam_deck, 0, tam_deck);

	pthread_mutex_init(&mutex_pa, NULL);
}

/*
Impede que novos clientes sejam aceitos e bloqueia até que os clientes dentro da pizzeria saiam voluntariamente.
Todo cliente que já estava sentado antes do fechamento, tem direito a receber e comer pizzas pendentes e a fazer novos pedidos.
Clientes que ainda não se sentaram não conseguirão sentar pois pegar_mesas retornará -1.
Chamada pela função main() antes de chamar pizzeria_destroy() e terminar o programa.
*/
void pizzeria_close() {

}

/*
Libera quaisquer recursos e estruturas de dados inicializados por pizzeria_init().
Chamada pelafunção main() antes de sair.
*/
void pizzeria_destroy() {

}

/*
Indica que a pizza dada como argumento (previamente colocada no forno) está pronta.
Chamada pelo nariz do pizzaiolo.
A thread que chamará essa função será uma thread específica para esse fim, criada nas profundezas do helper.c.
*/
void pizza_assada(pizza_t* pizza) {
	pizza->assada = 1;
}

/*
TRAICOEIRA
Algoritmo para conseguir mesas suficientes para um grupo de tam_grupo pessoas. Note que vários clientes podem chamar essa função ao mesmo tempo.
Deve retornar zero se não houve erro, ou -1 se a pizzaria já foi fechada com pizzeria_fechar().
A implementação não precisa considerar o layout das mesas.
Chamada pelo cliente líder do grupo.
*/
int pegar_mesas(int tam_grupo) {
    return -1; //erro: não fui implementado (ainda)!
}

/*
TRAICOEIRA
Indica que o grupo vai embora.
Chamada pelo cliente líder antes do grupo deixar a pizzaria.
*/
void garcom_tchau(int tam_grupo) {

}

/*
Chama um garçom, bloqueia até o garçom chegar.
Chamada pelo cliente líder.*
*/
void garcom_chamar() {
	sem_wait(&sem_garcons);

	//todo
	sem_post(&sem_garcons);
}

/*
Faz um pedido de pizza. O pedido aparece como uma smart ficha no smart deck. É proibido fazer um novo pedido antes de receber a pizza.
Chamado pelo cliente líder.
*/
void fazer_pedido(pedido_t* pedido) {
	queue_push_back(&queue_pedidos, pedido);
}
/*
Pega uma fatia da pizza. Retorna 0 (sem erro) se conseguiu pegar a fatia, ou -1 (erro) se a pizza já acabou.
Chamada pelas threads representando clientes.
*/
int pizza_pegar_fatia(pizza_t* pizza) {
    return -1; // erro: não fui implementado (ainda)!
}
