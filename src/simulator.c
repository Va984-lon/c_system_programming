#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

#include "../include/common.h"
#include "../include/logger.h"
#include "../include/data_loader.h"

static volatile sig_atomic_t g_stop = 0;
static pthread_mutex_t g_file_lock = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_sale_counter = 0;

static void on_signal(int sig) { (void)sig; g_stop = 1; }

typedef struct { int thread_id; int product_id; char product_name[MAX_NAME_LEN]; } ThreadArg;

static void *seller_thread(void *arg) {
    ThreadArg *ta = (ThreadArg *)arg;
    while (!g_stop) {
        struct timespec ts = { 1, 0 }; 
        nanosleep(&ts, NULL); 
        if (g_stop) break;
        
        Sale s;
        s.id = __sync_fetch_and_add(&g_sale_counter, 1);
        s.timestamp = time(NULL);
        s.customer_id = (rand() % 4) + 1;
        s.product_id = ta->product_id;
        append_sale(SALES_FILE, &s, &g_file_lock);
    }
    return NULL;
}

int main() {
    create_directories(); 
    logger_init("logs/simulator.log");
    signal(SIGINT, on_signal); 
    srand(time(NULL));
    generate_test_data();

    ProductList products; 
    load_products(PRODUCTS_FILE, &products);
    
    int num_threads = products.count > 3 ? 3 : products.count;
    pthread_t threads[3]; 
    ThreadArg args[3];

    printf("--- ЗАПУСК СИМУЛЯТОРА (10 секунд) ---\n");
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i + 1; 
        args[i].product_id = products.items[i].id;
        snprintf(args[i].product_name, MAX_NAME_LEN, "%s", products.items[i].name);
        pthread_create(&threads[i], NULL, seller_thread, &args[i]);
    }

    for (int sec = 0; sec < 10 && !g_stop; sec++) sleep(1); 
    g_stop = 1;

    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
    printf("Симулятор завершил работу. Создано продаж: %lld\n", (long long)g_sale_counter);
    return 0;
}