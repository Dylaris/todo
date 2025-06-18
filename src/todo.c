#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "todo.h"

void init_todos(todos_t *todos)
{
    todos->items = NULL;
    todos->count = 0;
    todos->capacity = 0;
    todos->strtab.start = NULL;
    todos->strtab.end = NULL;

    FILE *fp = fopen(TODO_DATA_FILE, "rb");
    if (!fp) {
        fp = fopen(TODO_DATA_FILE, "wb");
        assert(fp != NULL);
        fwrite(&todos->count, sizeof(size_t), 1, fp);
    } else {
        deserialize(todos);
    }
    fclose(fp);
}

static void free_todo(todo_t *todo)
{
    free(todo->name);
    free(todo->info);
    free(todo->time);
}

void free_todos(todos_t *todos)
{
    for (size_t i = 0; i < todos->count; i++) {
        todo_t *todo = &todos->items[i];
        free_todo(todo);
    }
    free(todos->items);
    free(todos->strtab.start);

    memset(todos, 0, sizeof(todos_t));
}

todo_t *get_todo(todos_t *todos, const char *name)
{
    for (size_t i = 0; i < todos->count; i++) {
        todo_t *todo = &todos->items[i];
        if (strcmp(todo->name, name) == 0) {
            return todo;
        }
    }
    return NULL;
}

int append_todo(todos_t *todos, const char *name, const char *info)
{
    if (is_todo_exist(todos, name)) return 1;

    if (todos->capacity <= todos->count) {
        todos->capacity = (todos->capacity==0) ? 10 : 2*todos->capacity;
        todos->items = realloc(todos->items, sizeof(todo_t)*todos->capacity);
        assert(todos->items != NULL);
    }

    todo_t *todo = &todos->items[todos->count++];
    todo->status = TODO;
    todo->name = strdup(name ? name : "(null)");
    todo->info = strdup(info ? info : "(null)");

    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int year = local->tm_year + 1900;
    int month = local->tm_mon + 1;
    int day = local->tm_mday;
    char date_str[30];
    sprintf(date_str, "%04d-%02d-%02d", year, month, day);

    todo->time = strdup(date_str);

    return 0;
}

bool is_todo_exist(todos_t *todos, const char *name)
{
    name = name ? name : "(null)";
    for (size_t i = 0; i < todos->count; i++) {
        if (strcmp(todos->items[i].name, name) == 0) return true;
    }
    return false;
}

static char *status_to_string(todo_status_t status)
{
    switch (status) {
    case TODO:      return "\033[34mTODO\033[0m";
    case WORKING:   return "\033[33mWORKING\033[0m";
    case DONE:      return "\033[32mDONE\033[0m";
    case CANCELLED: return "\033[31mCANCELLED\033[0m";
    case POSTPONED: return "\033[35mPOSTPONED\033[0m";
    default:        return "\033[37mUNKNOWN\033[0m";
    }
}

static void print_todo(todo_t *todo)
{
    printf("[%s] %s <%s> '%s'\n", status_to_string(todo->status),
            todo->time, todo->name, todo->info);
}

void list_todos(todos_t *todos)
{
    for (size_t i = 0; i < todos->count; i++) {
        todo_t *todo = &todos->items[i];
        printf("<%ld> ", i+1);
        print_todo(todo);
    }
}

void remove_todo(todos_t *todos, const char *name)
{
    for (size_t i = 0; i < todos->count; i++) {
        todo_t *todo = &todos->items[i];
        if (strcmp(todo->name, name) == 0) {
            free_todo(todo);
            memcpy(todo, todo+1, sizeof(todo_t)*(todos->count-i-1));
            todos->count--;
            break;
        }
    }
}

typedef struct {
    size_t name_idx;
    size_t info_idx;
    size_t time_idx;
    todo_status_t status;
} data_t;

static void append_todo_to_strtab(todos_t *todos, todo_t *todo)
{
    size_t old_len = todos->strtab.end - todos->strtab.start;
    size_t len = old_len + strlen(todo->name) + 
        strlen(todo->info) + strlen(todo->time) + 3;
    
    todos->strtab.start = realloc(todos->strtab.start, len);
    assert(todos->strtab.start != NULL);
    todos->strtab.end = todos->strtab.start + old_len;

    memcpy(todos->strtab.end, todo->name, strlen(todo->name)+1);
    todos->strtab.end += strlen(todo->name) + 1;
    memcpy(todos->strtab.end, todo->info, strlen(todo->info)+1);
    todos->strtab.end += strlen(todo->info) + 1;
    memcpy(todos->strtab.end, todo->time, strlen(todo->time)+1);
    todos->strtab.end += strlen(todo->time) + 1;
}

void serialize(todos_t *todos)
{
    FILE *fp = fopen(TODO_DATA_FILE, "wb"); 
    assert(fp != NULL);

    if (todos->strtab.start) free(todos->strtab.start);
    todos->strtab.start = NULL;
    todos->strtab.end = NULL;

    fwrite(&todos->count, sizeof(size_t), 1, fp);
    for (size_t i = 0; i < todos->count; i++) {
        todo_t *todo = &todos->items[i];
        data_t data = {0};
        data.name_idx = todos->strtab.end - todos->strtab.start;
        data.info_idx = data.name_idx + strlen(todo->name) + 1;
        data.time_idx = data.info_idx + strlen(todo->info) + 1;
        data.status = todo->status;
        append_todo_to_strtab(todos, todo);

        fwrite(&data, sizeof(data_t), 1, fp);
    }
    fwrite(todos->strtab.start, 1, todos->strtab.end-todos->strtab.start, fp);

    fclose(fp);
}

void deserialize(todos_t *todos)
{
#define READ_STRING(idx)                        \
    ({                                          \
        long saved = ftell(fp);                 \
        fseek(fp, strtab_off+(idx), SEEK_SET);  \
        size_t length = 0;                      \
        while (fgetc(fp) != '\0') {             \
            length++;                           \
        }                                       \
        fseek(fp, strtab_off+(idx), SEEK_SET);  \
        char *res = (char *) malloc(length+1);  \
        assert(res != NULL);                    \
        fread(res, 1, length, fp);              \
        res[length] = '\0';                     \
        fseek(fp, saved, SEEK_SET);             \
        res;                                    \
    })

    FILE *fp = fopen(TODO_DATA_FILE, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: can't open %s\n", TODO_DATA_FILE);
        exit(1);
    }
    
    size_t data_count;
    fread(&data_count, sizeof(size_t), 1, fp);

    todos->count = data_count;
    todos->capacity = (todos->count==0) ? 10 : 2*todos->count;
    todos->items = malloc(sizeof(todo_t)*todos->capacity);
    assert(todos->items != NULL);

    size_t strtab_off = sizeof(size_t) + sizeof(data_t)*data_count;

    for (size_t i = 0; i < data_count; i++) {
        data_t data;
        fread(&data, sizeof(data_t), 1, fp);
        todo_t todo = {
            .name = READ_STRING(data.name_idx),
            .info = READ_STRING(data.info_idx),
            .time = READ_STRING(data.time_idx),
            .status = data.status
        };
        todos->items[i] = todo;
    }

    fclose(fp);

#undef READ_STRING
}
