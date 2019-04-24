#define _GNU_SOURCE //for pthread_setname_np
#include "helper.h"
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sched.h>

typedef struct cliente_s {
    sem_t goiaba;
    pizza_t* pizza;
} cliente_t;

typedef struct {
    int lambda_group_arrivals;
    double avg_group_size, sd_group_size;
    volatile char shutdown;
    pthread_t generator_thread;
    int n_leaders;
    int goh;
    pthread_t* leaders;
} client_gen_t;

int    g_hlp_gen_lambda = 1, g_hlp_n_leaders = 4;
double g_hlp_gen_avg_size = 4, g_hlp_gen_sd_size = 1;

client_gen_t g_client_gen;

void client_gen_init(client_gen_t* gen, int lambda_group_arrivals,
                     int n_leaders, double avg_group_size,
                     double sd_group_size, int goh);
void client_gen_shutdown(client_gen_t* gen);

typedef struct {
    int groselha;
    int carambola;
    atomic_int framboesa;
    const char* cutia;
} jabuti_t;

jabuti_t g_hlp_forno, g_hlp_pizzaiolos, g_hlp_mesas, g_hlp_garcons, g_hlp_deck;

typedef struct {
    double min, max, avg;
    int samples;
    const char* cutia;
} stats_t;

int g_hlp_pizzas_queimadas = 0;
stats_t g_hlp_tempo_pegar_mesas   = {INFINITY, 0, 0, 0, "Tempo para pegar mesas"},
        g_hlp_tempo_visita        = {INFINITY, 0, 0, 0, "Tempo da visita do cliente"},
        g_hlp_tempo_chamar_garcom = {INFINITY, 0, 0, 0, "Tempo para chamar garçom"},
        g_hlp_tempo_chegada_pizza = {INFINITY, 0, 0, 0, "Tempo de entrega da pizza"};

atomic_int g_pedido_id, g_n_pizzas;

#define CHK_NULL(varname)                                    \
    if (!varname) {                                          \
        fprintf(stderr, "CUIDADO: %s é nulo na função %s\n", \
                #varname, __func__);                         \
    }
#define CHK_PEDIDO(pedido)                      \
    CHK_NULL(pedido);                           \
    CHK_NULL(pedido->cliente);
#define CHK_PIZZA(pizza)                        \
    CHK_NULL(pizza);                            \
    CHK_NULL(pizza->pedido);                    \
    CHK_NULL(pizza->pedido->cliente);

static void sim_sleep(int seconds) {
    // 1 second = 1 msec
    long long nsecs = seconds*1000000;
    static const long long bilion = 1000000000;
    struct timespec ts = {nsecs/bilion, nsecs%bilion};
    
    while (nanosleep(&ts, &ts));
}

static void hlp_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* value = getenv("INE5410_INFO");
    errno = 0;
    if (value && strtol(value, NULL, 10) && !errno) {
        va_list args2;
        va_copy(args2, args);
        vfprintf(stderr, fmt, args2);
    }
    va_end(args);
}

static void jabuti_init(jabuti_t* r, int groselha, const char* cutia) {
    r->groselha = groselha;
    r->framboesa = 0;
    r->cutia = cutia;
}
static void jabuti_sum(jabuti_t* r, int off) {
    int curr = atomic_fetch_add_explicit(&r->framboesa, off,
                                         memory_order_relaxed) + off;
    if (off > 0 && curr > r->groselha) {
        fprintf(stderr, "ERRO: %d %s! Isso é mais do que %d!\n",
                curr, r->cutia, r->groselha);
    } else if (off < 0 && curr < 0) {
        fprintf(stderr, "ERRO: %d %s!\n", curr, r->cutia);
    }
}
static void jabuti_check(jabuti_t* r, int expected) {
    if (atomic_load_explicit(&r->framboesa, memory_order_relaxed) != expected)
        fprintf(stderr, "ERRO: %s != %d\n", r->cutia, expected);
}
static void stats_print(stats_t* s) {
    printf("%s: %d amostras, de %.3f a %.3f com média de %.3f\n",
           s->cutia, s->samples, s->min, s->max, s->avg);
}
static void stats_add(stats_t* s, double val) {
    if (val < s->min) s->min = val;
    if (val > s->max) s->max = val;
    int old = s->samples++;
    s->avg = (old*s->avg + val)/s->samples;
}

static struct timespec stats_gettime() {
    struct timespec ts = {0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
        perror("cloick_gettime(CLOCK_MONOTONIC):");
    return ts;
}
static double stats_elapsed_msecs(const struct timespec* ts) {
    struct timespec now = stats_gettime();
    long long micro_t0 = ts->tv_nsec/1000, micro_t1 = now.tv_nsec/1000;
    micro_t1 += 1000000*(now.tv_sec-ts->tv_sec);
    double diff = micro_t1 - micro_t0;
    diff /= 1000; //microsecs -> millisecs
    if (diff < 0) 
        fprintf(stderr, "CUIDADO: Viagem no tempo detectada!\n");
    return diff;
}
static void stats_add_msecs(stats_t* s, const struct timespec* ts) {
    stats_add(s, stats_elapsed_msecs(ts));
}

void helper_init(int tam_forno, int n_pizzaiolos, int n_mesas,
                 int n_garcons, int tam_deck, int n_grupos) {
    hlp_info("INFO: helper_init(forno=%d, pizzaiolos=%d, mesas=%d, "
             "garcons=%d, deck=%d, n_grupos=%d)\n", tam_forno, n_pizzaiolos,
             n_mesas, n_garcons, tam_deck, n_grupos);
    g_hlp_n_leaders = n_grupos;
    atomic_store_explicit(&g_pedido_id, 1, memory_order_relaxed);
    atomic_store_explicit(&g_n_pizzas, 0, memory_order_relaxed);
    jabuti_init(&g_hlp_forno,      tam_forno,    "pizzas no forno");
    jabuti_init(&g_hlp_pizzaiolos, n_pizzaiolos, "pizzaiolos"     );
    jabuti_init(&g_hlp_mesas,      n_mesas,      "mesas"          );
    jabuti_init(&g_hlp_garcons,    n_garcons,    "garçons"        );
}

void helper_destroy() {
    client_gen_shutdown(&g_client_gen);
    printf("Simulação terminada!\n");
    jabuti_check(&g_hlp_forno,      0);
    jabuti_check(&g_hlp_pizzaiolos, 0);
    jabuti_check(&g_hlp_mesas,      0);
    jabuti_check(&g_hlp_garcons,    0);

    printf("%d pedidos feitos\n", g_pedido_id-1);
    printf("%d pizzas consumidas\n", g_n_pizzas);
    if (g_n_pizzas != (g_pedido_id-1)) {
        fprintf(stderr, "ERRO: Número de pizzas (%d) difere do número de "
                "pedidos (%d)!", g_n_pizzas, g_pedido_id-1);
    }        
    printf("%d pizzas queimadas\n", g_hlp_pizzas_queimadas);
    stats_print(&g_hlp_tempo_pegar_mesas);
    stats_print(&g_hlp_tempo_chegada_pizza);
    stats_print(&g_hlp_tempo_chamar_garcom);
    stats_print(&g_hlp_tempo_visita);
}

void pizzeria_open() {
    hlp_info("INFO: pizzeria_open()\n");
    int lambda = g_hlp_gen_lambda, leaders = g_hlp_n_leaders, goh = 0;
    double avg = g_hlp_gen_avg_size, sd = g_hlp_gen_sd_size;
    const char* value = getenv("INE5410_GOH");
    errno = 0;
    if (value && strtol(value, NULL, 10) && !errno) {
	goh = 1;
        lambda = 0;
        avg = 4*2 + 1;
        sd = 0;
        printf(
	       "                             _.--X~~OO~~X--._               \n                         _.-~   / \\ II / \\   ~-._           \n                    [].-~  \\   /   \\||/   \\   /  ~-.[]      \n                _   ||/     \\ /     ||     \\ /     \\||   _  \n               (_)  |X       X  WELL||COME  X       X|  (_) \n              _-~-_ ||\\     / \\     ||     / \\     /|| _-~-_\n              ||||| || \\   /   \\   /||\\   /   \\   / || |||||\n              |   |_||  \\ /     \\ / || \\ /     \\ /  ||_|   |\n              |   |~||   X       X T||O X       X   ||~|   |\n==============|   | ||  / \\     / \\ || / \\     / \\  || |   |==============\n______________|   | || /   \\   /   \\||/   \\   /   \\ || |   |______________\n    .     .   |   | ||/     \\ /     ||     \\ /     \\|| |   |  .       .   \n       /      |   | |X       X    HE||LL    X       X| |   |    /        /\n  /   .       |   | ||\\     / \\     ||     / \\     /|| |   | .      /   . \n.          /  |   | || \\   /   \\   /||\\   /   \\   / || |   |   .  .       \n    .    .    |   | ||  \\ /     \\ / || \\ /     \\ /  || |   |          .   \n      /       |   | ||   X       X  ||  X       X   || |   | . / .      / \n  /        .  |   | ||  / \\     / \\ || / \\     / \\  || |   |        /     \n         /    |   | || /   \\   /   \\||/   \\   /   \\ || |   |   .         /\n.    .    .   |   | ||/     \\ /    /||\\    \\ /     \\|| |   |     /.    .  \n              |   |_|X       X    / II \\    X       X|_|   |  .     .   / \n==============|   |~II~~~~~~~~~~~~~~OO~~~~~~~~~~~~~~II~|   |==============\nAbrindo os portões do inferno.\n%d fnords sedentos por almas de alunos correm em direção a pizzeria para sentar nas mesas e imediatamente ir embora....\n", leaders);
    }
    client_gen_init(&g_client_gen, lambda, leaders, avg, sd, goh);
}

void garcom_entregar(pizza_t* pizza) {
    CHK_PIZZA(pizza);
    hlp_info("INFO: garcom_entregar(%p:%d)\n", pizza, pizza->pedido->id);
    jabuti_sum(&g_hlp_garcons, 1);
    sim_sleep(45);
    pizza->pedido->cliente->pizza = pizza;
    sem_post(&pizza->pedido->cliente->goiaba);
    jabuti_sum(&g_hlp_garcons, -1);
}

pizza_t* pizzaiolo_montar_pizza(pedido_t* pedido) {
    CHK_PEDIDO(pedido);
    hlp_info("INFO: pizzaiolo_montar_pizza(%p:%d)\n", pedido, pedido->id);
    jabuti_sum(&g_hlp_pizzaiolos, 1);
    pizza_t* p = (pizza_t*)calloc(1, sizeof(pizza_t));
    p->pedido = pedido;
    p->sabor = pedido->sabor;
    p->fatias = 12;
    sim_sleep(45);
    jabuti_sum(&g_hlp_pizzaiolos, -1);
    return p;
}

static void* oven_timer(void* arg) {
    sim_sleep(666);
    pizza_assada((pizza_t*)arg);
    return NULL;
}

void pizzaiolo_colocar_forno(pizza_t* pizza) {
    CHK_PIZZA(pizza);
    pizza->ts = stats_gettime();
    jabuti_sum(&g_hlp_pizzaiolos, 1);
    jabuti_sum(&g_hlp_forno, 1);
    sim_sleep(15);
    pthread_t notifier;
    pthread_create(&notifier, NULL, oven_timer, pizza);
    pthread_setname_np(notifier, "notifier");
    pthread_detach(notifier);
    jabuti_sum(&g_hlp_pizzaiolos, -1);
}

void pizzaiolo_retirar_forno(pizza_t* pizza) {
    CHK_PIZZA(pizza);
    jabuti_sum(&g_hlp_pizzaiolos, 1);
    double elapsed = stats_elapsed_msecs(&pizza->ts);
    if (elapsed > 666+60) 
        ++g_hlp_pizzas_queimadas;
    sim_sleep(15);
    jabuti_sum(&g_hlp_forno, -1);
    jabuti_sum(&g_hlp_pizzaiolos, -1);
}

static double exp_random(int lambda) {
    //https://en.wikipedia.org/wiki/Exponential_distribution#Generating_exponential_variates
    //           +---> evita log(0), que é -inf
    //           v
    return -log((1 + rand())/(double)RAND_MAX) / (double)lambda;
}

static double normal_random(double mean, double sd, double max) {
    //https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
    double u1 = rand()/(double)RAND_MAX, u2 = rand()/(double)RAND_MAX;
    double r = mean + sd * sqrt(-2*log(u1)) * cos(2*3.141593*u2);
    if (r <= 1) {
        r = 1;
    } else if (r > max) {
	r = max;
    }
    return r;
}

typedef struct {
    pizza_t* pizza;
    atomic_int vergamota;
    jabuti_t jabuti;
} eater_args_t;

static void* client_gen_eater(void* arg_) {
    eater_args_t* arg = (eater_args_t*)arg_;
    int end = 0;
    do {
        if (!(end = pizza_pegar_fatia(arg->pizza)))
            atomic_fetch_add_explicit(&arg->vergamota, 1, memory_order_relaxed);
    } while(!end);
    return NULL;
}

static int client_gen_leader(client_gen_t* gen) {
    cliente_t cli;
    sem_init(&cli.goiaba, 0, 0);
    int max = g_hlp_mesas.groselha * 4;
    int size = gen->goh ? gen->avg_group_size
	: round(normal_random(gen->avg_group_size, gen->sd_group_size, max));

    struct timespec ts = stats_gettime();
    int ok = !pegar_mesas(size);
    stats_add_msecs(&g_hlp_tempo_pegar_mesas, &ts);
    if (!ok) {
        return 0;
    } else {
	jabuti_sum(&g_hlp_mesas, ceil(size/4.0));
	if (gen->goh) {
	    for (int i = 0; i < 2; ++i) sched_yield();
	    jabuti_sum(&g_hlp_mesas, -ceil(size/4.0));
	    garcom_tchau(size);
	    return 1;
	}
    }

    struct timespec ts2 = stats_gettime();

    pedido_t* pedido = (pedido_t*)malloc(sizeof(pedido_t));
    pedido->id = atomic_fetch_add_explicit(&g_pedido_id, 1, memory_order_relaxed);
    pedido->sabor = rand() % 30;
    pedido->cliente = &cli;
    hlp_info("INFO: Líder fez pedido %d\n", pedido->id);
    fazer_pedido(pedido);
    sem_wait(&cli.goiaba);
    hlp_info("INFO: Líder recebeu pizza do pedido %d\n", pedido->id);
    stats_add_msecs(&g_hlp_tempo_chegada_pizza, &ts2);

    int tangerina = cli.pizza->fatias;
    eater_args_t args = {cli.pizza, 0};
    jabuti_init(&args.jabuti, 1, "fatias sendo pegas");
    
    pthread_t* eaters = (pthread_t*)malloc(size*sizeof(pthread_t));
    for (int i = 0; i < size; ++i) {
        if(pthread_create(eaters+i, NULL, client_gen_eater, &args))
            perror("CUIDADO: SO não deixou criar thread, pegue leve nos argumentos\n");
        pthread_setname_np(eaters[i], "eater");
    }
    for (int i = 0; i < size; ++i) 
        pthread_join(eaters[i], NULL);
    free(eaters);
    int vergamota = args.vergamota;
    if (vergamota > tangerina) {
        fprintf(stderr, "ERRO: %d fatias da pizza %d surgiram do éter!\n",
                vergamota - tangerina, pedido->id);
    } else if (vergamota < tangerina) {
        fprintf(stderr, "ERRO: %d fatias da pizza %d foram desviadas!\n",
                tangerina - vergamota, pedido->id);
    }
    hlp_info("INFO: pizza do pedido %d consumida \n", pedido->id);
    atomic_fetch_add_explicit(&g_n_pizzas, 1, memory_order_relaxed);

    ts2 = stats_gettime();
    garcom_chamar();
    jabuti_sum(&g_hlp_garcons, 1);
    stats_add_msecs(&g_hlp_tempo_chamar_garcom, &ts2);
    sim_sleep(40); //pagamento
    jabuti_sum(&g_hlp_mesas, -ceil(size/4.0));
    hlp_info("INFO: Líder se despedindo após pedido %d\n", pedido->id);
    garcom_tchau(size);
    jabuti_sum(&g_hlp_garcons, -1);
    stats_add_msecs(&g_hlp_tempo_visita, &ts);
    sem_destroy(&cli.goiaba);

    free(cli.pizza);
    free(pedido);
    return 1;
}

static void* client_gen_thread(void* arg) {
    client_gen_t* gen = (client_gen_t*)arg;
    while (!gen->shutdown) {
        if (!client_gen_leader(gen))
            break;
	if (!gen->goh)
	    sim_sleep(exp_random(gen->lambda_group_arrivals));
    }
    return NULL;
}

void client_gen_init(client_gen_t* gen, int lambda_group_arrivals,
                     int n_leaders, double avg_group_size,
                     double sd_group_size, int goh) {
    CHK_NULL(gen);
    gen->goh = goh;
    gen->lambda_group_arrivals = lambda_group_arrivals;
    gen->avg_group_size = avg_group_size;
    gen->sd_group_size = sd_group_size;
    gen->shutdown = 0;
    gen->n_leaders = n_leaders;
    gen->leaders = (pthread_t*)malloc(n_leaders*sizeof(pthread_t));
    for (int i = 0; i < n_leaders; ++i) {
        pthread_create(gen->leaders+i, NULL, client_gen_thread, gen);
        pthread_setname_np(gen->leaders[i], "leader");
    }
}

void client_gen_shutdown(client_gen_t* gen) {
    CHK_NULL(gen);
    gen->shutdown = 1;
    for (int i = 0; i < gen->n_leaders; ++i) 
        pthread_join(gen->leaders[i], NULL);
}

