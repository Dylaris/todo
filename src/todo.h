#ifndef TODO_H
#define TODO_H

#include <stddef.h>
#include <stdbool.h>

// #define TODO_DATA_FILE "/home/dylaris/.todo.dat"
#define TODO_DATA_FILE ".todo.dat"

typedef enum {
    TODO,
    WORKING,
    DONE,
    CANCELLED,
    POSTPONED,
} todo_status_t;

typedef struct {
    todo_status_t status;
    char *name;
    char *info;
    char *time;
} todo_t;

typedef struct {
    todo_t *items;
    size_t count;
    size_t capacity;
    struct {
        char *start;
        char *end;
    } strtab;
} todos_t;

void init_todos(todos_t *todos);
void free_todos(todos_t *todos);
void list_todos(todos_t *todos);
bool is_todo_exist(todos_t *todos, const char *name);
int append_todo(todos_t *todos, const char *name, const char *info);
void remove_todo(todos_t *todos, const char *name);
void serialize(todos_t *todos);
void deserialize(todos_t *todos);
todo_t *get_todo(todos_t *todos, const char *name);

#endif // TODO_H
