#include "todo.h"
#include "zst.h"

static todos_t todos = {0}; 

static todo_status_t string_to_status(const char *status)
{
    if (strcmp(status, "TODO") == 0) return TODO;
    if (strcmp(status, "WORKING") == 0) return WORKING;
    if (strcmp(status, "DONE") == 0) return DONE;
    if (strcmp(status, "CANCELLED") == 0) return CANCELLED;
    if (strcmp(status, "POSTPONED") == 0) return POSTPONED;
    return -1;
}

void define_flags(zst_cmdline_t *cmdl)
{
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "h", "Print this information");
    zst_cmdline_define_flag(cmdl, FLAG_SINGLE_ARG, "n", "Specific a todo name: -n <name>");
    zst_cmdline_define_flag(cmdl, FLAG_SINGLE_ARG, "a", "Append a todo: -a <info>");
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "d", "Delete a todo: -n <name> -d"); 
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "l", "List all todos: -l"); 
    zst_cmdline_define_flag(cmdl, FLAG_SINGLE_ARG, "u", "Update the status of todo: -n <name> -u <status>");
}

void callback(zst_cmdline_t *cmdl)
{
    bool flag_h  = zst_cmdline_isuse(cmdl, "h");
    bool flag_n  = zst_cmdline_isuse(cmdl, "n");
    bool flag_a  = zst_cmdline_isuse(cmdl, "a");
    bool flag_d  = zst_cmdline_isuse(cmdl, "d");
    bool flag_l  = zst_cmdline_isuse(cmdl, "l");
    bool flag_u  = zst_cmdline_isuse(cmdl, "u");

    char *todo_name = NULL;

    if (flag_h) zst_cmdline_usage(cmdl);
    if (flag_n) {
        zst_flag_t *flag = zst_cmdline_get_flag(cmdl, "n");
        zst_string_t *val = zst_dyna_get(&flag->vals, 0);
        todo_name = val->base;
    }
    if (flag_d) {
        if (!todo_name) {
            zst_log(LOG_ERROR, "you need to specific a todo name");
            return;
        }
        remove_todo(&todos, todo_name);
    }
    if (flag_l) list_todos(&todos);
    if (flag_a) {
        if (!todo_name) {
            zst_log(LOG_ERROR, "you need to specific a todo name");
            return;
        }
        zst_flag_t *flag = zst_cmdline_get_flag(cmdl, "a");
        zst_string_t *val = zst_dyna_get(&flag->vals, 0);
        append_todo(&todos, todo_name, val->base);
    }
    if (flag_u) {
        if (!todo_name) {
            zst_log(LOG_ERROR, "you need to specific a todo name");
            return;
        }
        todo_t *todo = get_todo(&todos, todo_name);
        if (todo) {
            zst_flag_t *flag = zst_cmdline_get_flag(cmdl, "u");
            zst_string_t *val = zst_dyna_get(&flag->vals, 0);
            int status = string_to_status(val->base);
            if (status == -1) {
                fprintf(stderr, "ERROR: unknown status\n");
                return;
            }
            todo->status = status;
        }
    }
}

int main(int argc, char **argv)
{
    init_todos(&todos);

    zst_cmdline_t cmdl = {0};
    zst_cmdline_init(&cmdl);
    define_flags(&cmdl);

    zst_cmdline_parse(&cmdl, argc, argv);

    callback(&cmdl);

    zst_cmdline_free(&cmdl);

    serialize(&todos);
    free_todos(&todos);

    return 0;
}
