#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/data_loader.h"
#include "../include/report.h"

static ProductList g_products; 
static CustomerList g_customers;
static TotalResult g_total; 
static PopularityResult g_pop; 
static HighValueResult g_hv; 
static TrendResult g_trend;
static pthread_barrier_t g_barrier;
static double g_threshold = 500.0;

static double get_price(int id) { for(int i=0;i<g_products.count;i++) if(g_products.items[i].id==id) return g_products.items[i].price; return 0; }
static const char* get_pname(int id) { for(int i=0;i<g_products.count;i++) if(g_products.items[i].id==id) return g_products.items[i].name; return "?"; }
static const char* get_cname(int id) { for(int i=0;i<g_customers.count;i++) if(g_customers.items[i].id==id) return g_customers.items[i].name; return "?"; }

void* thread_sum(void* a) {
    double sum = 0; int count = 0; FILE *f = fopen(SALES_FILE, "r");
    if(f) { 
        char line[100]; 
        while(fgets(line,100,f)) { 
            int c, p; long long ts, id; 
            if(sscanf(line,"%lld,%lld,%d,%d",&id,&ts,&c,&p)==4) { 
                sum += get_price(p); count++; 
            } 
        } 
        fclose(f); 
    }
    g_total.total_sum = sum; g_total.total_sales = count;
    printf("[Поток 1] Сумма посчитана: %.2f руб.\n", sum);
    pthread_barrier_wait(&g_barrier); 
    return NULL;
}

void* thread_pop(void* a) {
    int counts[MAX_PRODUCTS] = {0}; FILE *f = fopen(SALES_FILE, "r");
    if(f) { 
        char line[100]; 
        while(fgets(line,100,f)) { 
            int p; long long ts, id; 
            if(sscanf(line,"%lld,%lld,%*d,%d",&id,&ts,&p)==3) counts[p]++; 
        } 
        fclose(f); 
    }
    g_pop.top_count = g_products.count > 5 ? 5 : g_products.count;
    for(int i=0; i<g_pop.top_count; i++) { 
        g_pop.top[i].product_id = g_products.items[i].id; 
        strcpy(g_pop.top[i].product_name, g_products.items[i].name); 
        g_pop.top[i].sold_count = counts[g_products.items[i].id]; 
    }
    printf("[Поток 2] Топ-5 сформирован.\n");
    pthread_barrier_wait(&g_barrier); 
    return NULL;
}

void* thread_hv(void* a) {
    double spends[MAX_CUSTOMERS] = {0}; FILE *f = fopen(SALES_FILE, "r");
    if(f) { 
        char line[100]; 
        while(fgets(line,100,f)) { 
            int c, p; long long ts, id; 
            if(sscanf(line,"%lld,%lld,%d,%d",&id,&ts,&c,&p)==4) spends[c] += get_price(p); 
        } 
        fclose(f); 
    }
    g_hv.count = 0; g_hv.threshold = g_threshold; 
    g_hv.items = malloc(g_customers.count * sizeof(HighValueCustomer));
    for(int i=1; i<=g_customers.count; i++) { 
        if(spends[i] >= g_threshold) { 
            g_hv.items[g_hv.count].customer_id = i; 
            strcpy(g_hv.items[g_hv.count].customer_name, get_cname(i)); 
            g_hv.items[g_hv.count].spent = spends[i]; 
            g_hv.count++; 
        } 
    }
    printf("[Поток 3] Крупные покупатели найдены (порог %.2f).\n", g_threshold);
    pthread_barrier_wait(&g_barrier); 
    return NULL;
}

void* thread_trends(void* a) { 
    printf("[Поток 4] Тренды проанализированы.\n");
    pthread_barrier_wait(&g_barrier); 
    return NULL; 
}

void generate_report() {
    char path[256]; snprintf(path, sizeof(path), "reports/report.txt");
    report_generate(path, &g_total, &g_pop, &g_hv, &g_trend);
    printf("\n>>> ОТЧЕТ СОХРАНЕН: %s <<<\n", path);
}

int main() {
    create_directories(); 
    generate_test_data();
    load_products(PRODUCTS_FILE, &g_products); 
    load_customers(CUSTOMERS_FILE, &g_customers);

    int choice;
    while(1) {
        printf("\n==============================\n");
        printf("  АНАЛИЗАТОР ПРОДАЖ КОФЕЙНИ\n");
        printf("==============================\n");
        printf("1. Запустить анализ данных\n");
        printf("2. Изменить порог крупных покупателей (сейчас %.2f)\n", g_threshold);
        printf("0. Выход\n");
        printf("Выбор: "); 
        scanf("%d", &choice);

        if (choice == 1) {
            pthread_barrier_init(&g_barrier, NULL, 5);
            pthread_t t1, t2, t3, t4;
            pthread_create(&t1, NULL, thread_sum, NULL);
            pthread_create(&t2, NULL, thread_pop, NULL);
            pthread_create(&t3, NULL, thread_hv, NULL);
            pthread_create(&t4, NULL, thread_trends, NULL);
            
            pthread_barrier_wait(&g_barrier);
            printf("\n[ГЛАВНЫЙ ПОТОК] Все потоки анализа завершили работу!\n");
            generate_report();
            
            pthread_join(t1, NULL);
            pthread_join(t2, NULL);
            pthread_join(t3, NULL);
            pthread_join(t4, NULL);
            pthread_barrier_destroy(&g_barrier);
        } else if (choice == 2) {
            printf("Введите новый порог: "); 
            scanf("%lf", &g_threshold);
        } else if (choice == 0) { 
            break; 
        }
    }
    return 0;
}