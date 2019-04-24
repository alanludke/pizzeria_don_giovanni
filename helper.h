#ifndef __HELPER_H_
#define __HELPER_H_

#include "pizzeria.h"

void helper_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                 int n_garcons, int tam_deck, int n_grupos);
void helper_destroy();
void pizzeria_open();
void garcom_entregar(pizza_t* pizza);
pizza_t* pizzaiolo_montar_pizza(pedido_t* pedido);
void pizzaiolo_colocar_forno(pizza_t* pizza);
void pizzaiolo_retirar_forno(pizza_t* pizza);


#endif /*__HELPER_H_*/
