#include "../include/report.h"
#include <stdio.h>

int report_generate(const char *path, const TotalResult *total, const PopularityResult *pop, const HighValueResult *hv, const TrendResult *trend) {
    FILE *f = fopen(path, "w"); if (!f) return -1;
    fprintf(f, "====================================\n");
    fprintf(f, "=== ОТЧЕТ ПО ПРОДАЖАМ КОФЕЙНИ =====\n");
    fprintf(f, "====================================\n\n");
    fprintf(f, "1. ОБЩАЯ ВЫРУЧКА: %.2f руб.\n", total->total_sum);
    fprintf(f, "   Всего продаж: %d\n\n", total->total_sales);
    
    fprintf(f, "2. ТОП-5 ТОВАРОВ:\n");
    for (int i = 0; i < pop->top_count; i++) {
        fprintf(f, "   %d. %-15s - %d шт.\n", i+1, pop->top[i].product_name, pop->top[i].sold_count);
    }
    
    fprintf(f, "\n3. КРУПНЫЕ ПОКУПАТЕЛИ (от %.2f руб.):\n", hv->threshold);
    for (int i = 0; i < hv->count; i++) {
        fprintf(f, "   %-15s - %.2f руб.\n", hv->items[i].customer_name, hv->items[i].spent);
    }
    fclose(f); return 0;
}