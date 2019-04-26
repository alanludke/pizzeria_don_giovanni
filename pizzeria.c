#include "pizzeria.h"
#include "queue.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

/*
Inicializa quaisquer recursos e estruturas de dados que sejam necessários antes da pizzeria poder receber clientes.
Chamada pela função main() antes de qualquer outra função.
*/
void pizzeria_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                   int n_garcons, int tam_deck, int n_grupos) {
	


	//loop para criar as threads de pizzaiolos
	for (int i = 0; i < n_pizzaiolos; i++) {

	    pthread_create(&pizzaiolos[i], NULL, sumVectors, NULL);

	}

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

}

/*
Faz um pedido de pizza. O pedido aparece como uma smart ficha no smart deck. É proibido fazer um novo pedido antes de receber a pizza.
Chamado pelo cliente líder.
*/
void fazer_pedido(pedido_t* pedido) {

}

/*
Pega uma fatia da pizza. Retorna 0 (sem erro) se conseguiu pegar a fatia, ou -1 (erro) se a pizza já acabou.
Chamada pelas threads representando clientes.
*/
int pizza_pegar_fatia(pizza_t* pizza) {
    return -1; // erro: não fui implementado (ainda)!
}
