#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <time.h>
#include <pthread.h>

#define MAX_PRODUCTS    50
#define MAX_CUSTOMERS   100
#define MAX_SALES       10000
#define MAX_NAME_LEN    64
#define MAX_PATH_LEN    256
#define MAX_LINE_LEN    512

#define SALES_FILE      "data/sales.csv"
#define PRODUCTS_FILE   "data/products.csv"
#define CUSTOMERS_FILE  "data/customers.csv"
#define REPORTS_DIR     "reports"
#define LOGS_DIR        "logs"

typedef struct { int id; char name[MAX_NAME_LEN]; double price; } Product;
typedef struct { int id; char name[MAX_NAME_LEN]; } Customer;
typedef struct { int64_t id; time_t timestamp; int customer_id; int product_id; } Sale;

typedef struct { Product items[MAX_PRODUCTS]; int count; } ProductList;
typedef struct { Customer items[MAX_CUSTOMERS]; int count; } CustomerList;

void ensure_directory(const char *path);
int  create_directories(void);

#endif