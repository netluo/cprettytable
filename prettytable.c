//
// Created by luocx on 4/26/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "prettytable.h"


// 创建新行
Row create_row(int num_cells, int max_width) {
    Row new_row;
    new_row.cells = (char **) malloc(sizeof(char *) * num_cells);
    new_row.num_cells = num_cells;
    for (int i = 0; i < num_cells; i++) {
        new_row.cells[i] = (char *) malloc(sizeof(char) * (max_width + 1));
        memset(new_row.cells[i], '\0', sizeof(char) * (max_width + 1));
    }
    return new_row;
}

// 初始化表格
void init_table(Table *table, char **headers, int num_headers, int max_width) {
    table->headers = headers;
    table->num_headers = num_headers;
    table->rows = NULL;
    table->num_rows = 0;
    table->max_width = max_width;  // 列宽度限制
}


// 在表格末尾添加新行
void add_row(Table *table, Row row) {
    Row *new_rows = (Row *) realloc(table->rows, sizeof(Row) * (table->num_rows + 1));
    if (new_rows == NULL) {
        printf("Error: Failed to allocate memory\n");
        return;
    }
    table->rows = new_rows;
    table->rows[table->num_rows++] = row;

    // 释放旧的内存
    // 注意：只有在 realloc() 返回非空指针时才需要释放旧的内存
    if (new_rows != table->rows) {
        free(table->rows);
    }
}

// 打印表格
void print_table(Table *table) {
    // 计算每列的最大宽度
    int *widths = (int *) malloc(sizeof(int) * table->num_headers);
    memset(widths, 0, sizeof(int) * table->num_headers);
    for (int i = 0; i < table->num_headers; i++) {
        widths[i] = (int) strlen(table->headers[i]);
    }
    for (int i = 0; i < table->num_rows; i++) {
        for (int j = 0; j < table->rows[i].num_cells; j++) {
            int cell_width = (int) strlen(table->rows[i].cells[j]);
            if (cell_width > table->max_width) {
                cell_width = table->max_width;
                table->rows[i].cells[j][table->max_width] = '\0';  // 超过宽度限制的部分截断
            }
            if (cell_width > widths[j]) {
                widths[j] = cell_width;
            }
        }
    }
    // 打印分隔线
    printf("+");
    for (int i = 0; i < table->num_headers; i++) {
        for (int j = 0; j < widths[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }
    printf("\n");

    // 打印表头
    printf("|");
    for (int i = 0; i < table->num_headers; i++) {
        printf(" %-*s |", widths[i], table->headers[i]);
    }
    printf("\n");

    // 打印分隔线
    printf("+");
    for (int i = 0; i < table->num_headers; i++) {
        for (int j = 0; j < widths[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }
    printf("\n");

    // 打印数据行
    for (int i = 0; i < table->num_rows; i++) {
        printf("|");
        for (int j = 0; j < table->rows[i].num_cells; j++) {
            printf(" %-*s |", widths[j], table->rows[i].cells[j]);
        }
        printf("\n");
    }

    // 打印分隔线
    printf("+");
    for (int i = 0; i < table->num_headers; i++) {
        for (int j = 0; j < widths[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }
    printf("\n");

    // 释放内存
    free(widths);
}

// 释放表格所占用的内存
void free_table(Table *table) {
    for (int i = 0; i < table->num_rows; i++) {
        for (int j = 0; j < table->rows[i].num_cells; j++) {
            free(table->rows[i].cells[j]);
        }
        free(table->rows[i].cells);
    }
    if (table->rows != NULL) {
        free(table->rows);
    }
}

// from mysql or other connection
Table from_cursor(MYSQL_RES *result, int max_width) {
    // 构建表格
    int num_fields = (int) mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    char **headers = (char **) malloc(sizeof(char *) * num_fields);
    if (headers == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        mysql_free_result(result);
        exit(-1);
    }

    for (int i = 0; i < num_fields; i++) {
        headers[i] = fields[i].name;
        // print every line seq:name:type
        // printf("%d:%s:%u\n", i + 1, fields[i].name, fields[i].type);
    }

    Table table;
    init_table(&table, headers, num_fields, max_width);

    // 解析结果集的行数据
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        Row new_row = create_row(num_fields, max_width);
        for (int i = 0; i < num_fields; i++) {
            if (row[i] == NULL) {
                printf("NULL \n");
                strncpy(new_row.cells[i], (const char *) "NULL", strlen("NULL"));
            } else {
                char buf[max_width + 1];
                sprintf(buf, "%s", row[i]); // 格式化为字符串后输出
                strncpy(new_row.cells[i], row[i], max_width);
            }
        }
        add_row(&table, new_row);
    }
    return table;
}