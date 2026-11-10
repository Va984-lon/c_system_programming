#include "../include/data_loader.h"
#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int file_exists(const char *path) { struct stat st; return stat(path, &st) == 0; }
static void trim_newline(char *s) { size_t len = strlen(s); while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) s[--len] = '\0'; }
static void skip_header(FILE *f) { char line[MAX_LINE_LEN]; if (fgets(line, sizeof(line), f)) { if (strncmp(line, "id", 2) != 0) rewind(f); } }

int load_products(const char *path, ProductList *out) {
    FILE *f = fopen(path, "r"); if (!f) return -1;
    char line[MAX_LINE_LEN]; out->count = 0; skip_header(f);
    while (fgets(line, sizeof(line), f) && out->count < MAX_PRODUCTS) {
        trim_newline(line); if (strlen(line) == 0) continue;
        Product *p = &out->items[out->count];
        if (sscanf(line, "%d,%63[^,],%lf", &p->id, p->name, &p->price) == 3) out->count++;
    }
    fclose(f); return 0;
}

int load_customers(const char *path, CustomerList *out) {
    FILE *f = fopen(path, "r"); if (!f) return -1;
    char line[MAX_LINE_LEN]; out->count = 0; skip_header(f);
    while (fgets(line, sizeof(line), f) && out->count < MAX_CUSTOMERS) {
        trim_newline(line); if (strlen(line) == 0) continue;
        Customer *c = &out->items[out->count];
        if (sscanf(line, "%d,%63[^\n]", &c->id, c->name) == 2) out->count++;
    }
    fclose(f); return 0;
}

int append_sale(const char *path, const Sale *sale, pthread_mutex_t *lock) {
    if (lock) pthread_mutex_lock(lock);
    FILE *f = fopen(path, "a");
    if (!f) { if (lock) pthread_mutex_unlock(lock); return -1; }
    fprintf(f, "%lld,%lld,%d,%d\n", (long long)sale->id, (long long)sale->timestamp, sale->customer_id, sale->product_id);
    fclose(f); if (lock) pthread_mutex_unlock(lock); return 0;
}

int generate_test_data(void) {
    if (!file_exists(PRODUCTS_FILE)) {
        FILE *f = fopen(PRODUCTS_FILE, "w"); fprintf(f, "id,name,price\n");
        fprintf(f, "1,Капучино,250.00\n2,Латте,280.00\n3,Американо,200.00\n4,Круассан,150.00\n5,Чизкейк,300.00\n");
        fclose(f);
    }
    if (!file_exists(CUSTOMERS_FILE)) {
        FILE *f = fopen(CUSTOMERS_FILE, "w"); fprintf(f, "id,name\n");
        fprintf(f, "1,Иванов И.И.\n2,Петров П.П.\n3,Смирнова А.В.\n4,Кузнецов В.И.\n");
        fclose(f);
    }
    if (!file_exists(SALES_FILE)) { FILE *f = fopen(SALES_FILE, "w"); fprintf(f, "id,timestamp,customer_id,product_id\n"); fclose(f); }
    return 0;
}