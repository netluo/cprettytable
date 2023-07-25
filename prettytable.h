//
// Created by luocx on 4/26/23.
//

#ifndef MYSQLSCAN_PRETTYTABLE_H
#define MYSQLSCAN_PRETTYTABLE_H

// 定义表格行结构体
typedef struct row {
    char **cells;
    int num_cells;
} Row;

// 定义表格结构体
typedef struct table {
    char **headers;
    int num_headers;
    Row *rows;
    int num_rows;
    int max_width;
} Table;

extern Row create_row(int num_cells, int max_width);

extern void init_table(Table *table, char **headers, int num_headers, int max_width);

extern void add_row(Table *table, Row row);

extern void print_table(Table *table);

extern void free_table(Table *table);

extern Table from_cursor(MYSQL_RES *result, int max_width);

#endif //MYSQLSCAN_PRETTYTABLE_H
