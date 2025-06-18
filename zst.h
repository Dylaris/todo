/*
 * Author: Dylaris
 * Copyright (c) 2025
 * License: MIT
 * Date: 2025-06-08
 *
 * All rights reserved
 */

/*
 * Usage: Define a few macros to enable the corresponding functions
 *
 *   ZST_STATIC            Restricts the function scope to static
 *   ZST_IMPLEMENTATION    Includes function implementation when defined, otherwise only the header is included
 *   ZST_STRIP_OUT         Strip out prefix 'zst'
 *
 * '+' means you can use it in 'linux' and 'windows', otherwise only 'linux'
 *
 * + ZST_FORGER            Build the c project using only c 
 * + ZST_PROCESS           Process operations
 * + ZST_LOG               Simple logging for information
 * + ZST_FILE_SYSTEM       Some operations about file and directory
 * + ZST_WILDCARD          Wildcard ( '*', '?' )
 * + ZST_FLAG_PARSER       Some operations about command line (flag, ...)
 * + ZST_DS_DYNAMIC_ARRAY  Dynamic array
 * + ZST_DS_STRING         String
 * + ZST_DS_STACK          Stack
 * + ZST_DS_QUEUE          Queue (based on linked-list)
 *
 * !!! NOTE !!!
 *
 * Since `zst.h` implements a generic data structure, the elements you push into it should be pointers. 
 * For example, if you want to push an integer, you should pass an (int *). 
 *
 * Because strings are not of fixed length, they cannot be directly used as elements of these generic data structures. 
 * Therefore, we use (char **) as the element type of the data structure, pointing to the string. 
 * Since what we actually want to push into the data structure is (char **), we need to pass (char ***).
 * If you find this cumbersome, you can simply use the implemented zst_string_t.
 */

#ifndef ZERO_START_H
#define ZERO_START_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#ifdef ZST_STATIC
#define ZST_DEF static
#else
#define ZST_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

/* MODULE: ZST_WILDCARD */
ZST_DEF bool zst_wildcard_match(const char *str, const char *pattern);

/* MODULE: ZST_LOG */
#define LOG_INFO  1
#define LOG_ERROR 2
#define LOG_GOOD  3
#define LOG_TODO  4
#define LOG_FATAL 5
ZST_DEF void zst_log(int type, const char *fmt, ...);

/* MODULE: ZST_DS_DYNAMIC_ARRAY */

typedef struct {
    void *base;
    size_t count;
    size_t capacity;
    size_t size; // size of each element
    void (*free_item)(void *);
} zst_dyna_t;

ZST_DEF void zst_dyna_init(zst_dyna_t *da, size_t size, void (*free_item)(void *));
ZST_DEF void zst_dyna_append(zst_dyna_t *da, void *elem);
ZST_DEF void zst_dyna_insert(zst_dyna_t *da, size_t idx, void *elem);
ZST_DEF void zst_dyna_remove(zst_dyna_t *da, size_t idx);
ZST_DEF void *zst_dyna_get(zst_dyna_t *da, size_t idx);
ZST_DEF void zst_dyna_free(zst_dyna_t *da);
ZST_DEF void zst_dyna_merge(zst_dyna_t *to, zst_dyna_t *from);

/* MODULE: ZST_DS_STRING */

typedef struct {
    char *base;
    size_t length;
    size_t capacity;
} zst_string_t;

ZST_DEF void zst_string_append(zst_string_t *str, const char *fmt, ...);
ZST_DEF zst_string_t zst_string_sub(const char *str, size_t src, size_t dest);
ZST_DEF zst_string_t zst_string_repeat(const char *str, size_t times);
ZST_DEF zst_string_t zst_string_replace(const char *str, const char *s_old, const char *s_new);
ZST_DEF void zst_string_free(void *arg);
static zst_string_t zst_string_concat_(const char *delim, ...);

#ifdef ZST_IMPLEMENTATION
#define zst_string_concat(delim, ...) zst_string_concat_(delim, ##__VA_ARGS__, NULL)
#else
#define zst_string_concat(delim, ...)
#endif

/* MODULE: ZST_DS_STACK */

typedef struct {
    void *base;
    int top;
    size_t capacity;
    size_t size; // size of each element
    void (*free_item)(void *);
    zst_dyna_t gc;
} zst_stack_t;

ZST_DEF void zst_stack_init(zst_stack_t *stk, size_t size, void (*free_item)(void *));
ZST_DEF void zst_stack_push(zst_stack_t *stk, void *elem);
ZST_DEF void *zst_stack_pop(zst_stack_t *stk);
ZST_DEF void *zst_stack_top(zst_stack_t *stk);
ZST_DEF void zst_stack_free(zst_stack_t *stk);

#ifdef ZST_IMPLEMENTATION
#define zst_stack_isempty(stack) ((stack)->top == -1)
#else
#define zst_stack_isempty(stack)
#endif

/* MODULE: ZST_DS_QUEUE */

typedef struct zst_queue_node {
    void *data;
    struct zst_queue_node *next;
} zst_queue_node_t;

typedef struct {
    zst_queue_node_t head; // dummy
    zst_queue_node_t *front, *rear;
    size_t count;
    size_t size; // size of each element
    zst_dyna_t gc;
    void (*free_item)(void *);
} zst_queue_t;

#ifdef ZST_IMPLEMENTATION
#define zst_queue_isempty(queue) ((queue)->head.next == NULL)
#else
#define zst_queue_isempty(queue)
#endif

ZST_DEF void zst_queue_init(zst_queue_t *queue, size_t size, void (*free_item)(void *));
ZST_DEF void zst_queue_free(zst_queue_t *queue);
ZST_DEF void *zst_queue_front(zst_queue_t *queue);
ZST_DEF void *zst_queue_rear(zst_queue_t *queue);
ZST_DEF void zst_queue_push(zst_queue_t *queue, void *elem);
ZST_DEF void *zst_queue_pop(zst_queue_t *queue);


/* MODULE: ZST_FILE_SYSTEM */

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif // platform

/* file type */
#define FT_NOET 0    // not exist
#define FT_REG  1    // regular file
#define FT_DIR  2    // directory

/* file attribute */
#define FA_NONE  0
#define FA_EXEC  (1 << 0)
#define FA_READ  (1 << 1)
#define FA_WRITE (1 << 2)

#define FS_MAX_PATH_SIZE 1024

#define ZST_INVALID_FD -1

typedef int zst_fd_t;

#define ZST_STDIN  0
#define ZST_STDOUT 1
#define ZST_STDERR 2

typedef struct {
    char *name;
    char *content;
    int type;
    int attr;
    size_t size;
    size_t line;
} zst_meta_file_t ;

typedef struct {
    char *name;
    char **files;
    char **dirs;
    size_t f_cnt;
    size_t d_cnt;
    size_t count;
} zst_meta_dir_t;

/* FIXME: zst_fs_copy can't copy a directory A to another directory B.
   It just rename A to B (delete B) (the same to zst_fs_move). And also, 
   it seems stuck into a loop when operating the directory which it stays. */

ZST_DEF unsigned long long zst_fs_get_timestamp(const char *pathname);
ZST_DEF char *zst_fs_get_name(const char *pathname);
ZST_DEF bool zst_fs_pwd(char *buf, size_t buf_size);
ZST_DEF bool zst_fs_cd(const char *pathname);
ZST_DEF bool zst_fs_move(const char *src, const char *dest);
ZST_DEF bool zst_fs_copy(const char *src, const char *dest);
ZST_DEF bool zst_fs_mkdir(const char *pathname);
ZST_DEF bool zst_fs_touch(const char *pathname);
ZST_DEF bool zst_fs_remove(const char *pathname);
ZST_DEF bool zst_fs_remove_all(zst_dyna_t *items);
ZST_DEF zst_dyna_t zst_fs_match(const char *pathname, const char *pattern);
ZST_DEF zst_dyna_t zst_fs_find(const char *pathname, int attrs);
ZST_DEF zst_dyna_t zst_fs_match_recursively(const char *pathname, const char *pattern);
ZST_DEF zst_dyna_t zst_fs_find_recursively(const char *pathname, int attrs);
ZST_DEF int zst_fs_typeof(const char *pathname);
ZST_DEF void zst_fs_free_meta_file(zst_meta_file_t *mf);
ZST_DEF void zst_fs_free_meta_dir(zst_meta_dir_t *md);
ZST_DEF int zst_fs_get_attr(const char *filename);
ZST_DEF size_t zst_fs_get_size(const char *filename);
ZST_DEF bool zst_fs_check_perm(const char *filename, int perm);
ZST_DEF bool zst_fs_load_file(const char *filename, zst_meta_file_t *res);
ZST_DEF bool zst_fs_dump_file(const char *dest_file, const char *src_buf, size_t size);
ZST_DEF bool zst_fs_load_dir(const char *dirname, zst_meta_dir_t *res);

#ifdef ZST_IMPLEMENTATION
#ifdef _WIN32
#define zst_fs_open_to_read(name)       _open((name), _O_RDONLY | _O_BINARY)
#define zst_fs_open_to_write(name)      _open((name), _O_CREAT | _O_WRONLY | _O_BINARY, 0644)
#define zst_fs_open_to_append(name)     _open((name), _O_CREAT | _O_APPEND | _O_WRONLY | _O_BINARY, 0644)
#define zst_fs_close(fd)                _close(fd)
#define zst_fs_read(fd, buffer, size)   _read((fd), (buffer), (size))
#define zst_fs_write(fd, buffer, size)  _write((fd), (buffer), (size))
#define zst_fs_redirect(from, to)       do { _dup2((from), (to)); _close(from); } while (0)
#else
#define zst_fs_open_to_read(name)       open((name), O_RDONLY)
#define zst_fs_open_to_write(name)      open((name), O_CREAT | O_WRONLY, 0644)
#define zst_fs_open_to_append(name)     open((name), O_CREAT | O_APPEND | O_WRONLY, 0644)
#define zst_fs_close(fd)                close(fd)
#define zst_fs_read(fd, buffer, size)   read((fd), (buffer), (size))
#define zst_fs_write(fd, buffer, size)  write((fd), (buffer), (size))
#define zst_fs_redirect(from, to)       do { dup2((from), (to)); close(from); } while (0)
#endif // platform
#else
#define zst_fs_open_to_read(name)
#define zst_fs_open_to_write(name)
#define zst_fs_open_to_append(name)
#define zst_fs_read(fd, buffer, size)
#define zst_fs_write(fd, buffer, size)
#define zst_fs_close(fd)
#define zst_fs_redirect(from, to)
#endif

/* MODULE: ZST_FLAG_PARSER */

typedef enum {
    FLAG_NO_ARG = 0,
    FLAG_SINGLE_ARG,
    FLAG_MULTI_ARG,
    NR_FLAG,
} zst_flag_type_t;

typedef struct {
    bool isused;
    zst_flag_type_t type;
    zst_string_t name;
    zst_dyna_t vals;
    zst_string_t info;
} zst_flag_t;

typedef struct {
    zst_string_t prog;
    zst_dyna_t flags; // each element is 'zst_flag_t'
} zst_cmdline_t;

ZST_DEF void zst_cmdline_init(zst_cmdline_t *cmdl);
ZST_DEF void zst_cmdline_define_flag(zst_cmdline_t *cmdl, zst_flag_type_t type, const char *name, const char *info);
ZST_DEF void zst_cmdline_parse(zst_cmdline_t *cmdl, int argc, char **argv);
ZST_DEF void zst_cmdline_usage(zst_cmdline_t *cmdl);
ZST_DEF bool zst_cmdline_isuse(zst_cmdline_t *cmdl, const char *flagname);
ZST_DEF zst_flag_t *zst_cmdline_get_flag(zst_cmdline_t *cmdl, const char *flagname);
ZST_DEF void zst_cmdline_free(void *arg);
ZST_DEF void zst_cmdline_dump(zst_cmdline_t *cmdl);
ZST_DEF void zst_flag_init(zst_flag_t *flag);
ZST_DEF void zst_flag_dump(zst_flag_t *flag, size_t level);
ZST_DEF void zst_flag_free(void *arg);

/* MODULE: ZST_PROCESS */

/* MODULE: ZST_FORGER */

typedef enum {
    EXEC_UNDO = 0,
    EXEC_OK,
    EXEC_ERROR,
} zst_exec_status_t;

typedef struct {
    zst_dyna_t args; // each element is zst_string_t
    size_t count;
} zst_cmd_t;

typedef struct {
    zst_dyna_t cmds; // each element is zst_cmd_t
    size_t count;
    zst_string_t target;
    zst_dyna_t srcs;
    zst_dyna_t objs;
} zst_forger_t;

static void zst_cmd_append_arg_(zst_cmd_t *cmd, ...);
ZST_DEF void zst_cmd_init(zst_cmd_t *cmd);
ZST_DEF void zst_cmd_dump(zst_cmd_t *cmd);
ZST_DEF int zst_cmd_run(zst_cmd_t *cmd);
ZST_DEF void zst_cmd_free(zst_cmd_t *cmd);

#ifdef ZST_IMPLEMENTATION
#define zst_cmd_append_arg(cmd, ...) zst_cmd_append_arg_((cmd), ##__VA_ARGS__, NULL)
#else
#define zst_cmd_append_arg(cmd, ...)
#endif

static void zst_forger_append_cmd_(zst_forger_t *forger, ...);
static void zst_forger_update_self_(const char *cc, int argc, char **argv, const char *source, ...);
ZST_DEF void zst_forger_init(zst_forger_t *forger, const char *target, const char *dir, bool is_recursive);
ZST_DEF void zst_forger_free(zst_forger_t *forger);
ZST_DEF void zst_forger_dump(zst_forger_t *forger);
ZST_DEF int zst_forger_run_sync(zst_forger_t *forger);

#ifdef ZST_IMPLEMENTATION
#define zst_forger_update_self(cc, argc, argv, ...) zst_forger_update_self_(cc, argc, argv, __FILE__, ##__VA_ARGS__, NULL)
#else
#define zst_forger_update_self(cc, argc, argv, ...)
#endif

#ifdef ZST_IMPLEMENTATION
#define zst_forger_append_cmd(forger, ...) zst_forger_append_cmd_((forger), ##__VA_ARGS__, NULL)
#else
#define zst_forger_append_cmd(forger, ...)
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ZERO_START_H

#ifdef ZST_IMPLEMENTATION

/* MODULE: ZST_FLAG_PARSER */

ZST_DEF void zst_cmdline_dump(zst_cmdline_t *cmdl)
{
    fprintf(stderr, "<program>: %s\n", cmdl->prog.base);

    if (cmdl->flags.count > 0) {
        fprintf(stderr, "<flags>:\n");
        for (size_t i = 0; i < cmdl->flags.count; i++) {
            fprintf(stderr, "\tflags[%zu]:\n", i);
            zst_flag_t *flag = zst_dyna_get(&cmdl->flags, i);
            zst_flag_dump(flag, 2);
        }
    } else {
        fprintf(stderr, "<flags>: EMPTY\n");
    }
}

static inline void flag_type_to_string_(int type, char *buf, size_t n)
{
    switch (type) {
    case FLAG_NO_ARG:
        snprintf(buf, n, "NO ARG");
        break;
    case FLAG_SINGLE_ARG:
        snprintf(buf, n, "SINGLE ARG");
        break;
    case FLAG_MULTI_ARG:
        snprintf(buf, n, "MULTI ARG");
        break;
    default:
        snprintf(buf, n, "UNKNOWN");
        break;
    }
}

ZST_DEF void zst_flag_dump(zst_flag_t *flag, size_t level)
{
    zst_string_t indent = zst_string_repeat("\t", level);

    char type_str[20];
    flag_type_to_string_(flag->type, type_str, sizeof(type_str));

    fprintf(stderr, "%s<name>: %s\n", indent.base, flag->name.base);
    fprintf(stderr, "%s<type>: %s\n", indent.base, type_str);
    fprintf(stderr, "%s<used>: %s\n", indent.base, flag->isused ? "true" : "false");

    if (flag->vals.count > 0) {
        fprintf(stderr, "%s<vals>:\n", indent.base);
        for (size_t i = 0; i < flag->vals.count; i++) {
            zst_string_t *val = zst_dyna_get(&flag->vals, i);
            fprintf(stderr, "%s\tvals[%zu]: %s\n", indent.base, i, val->base);
        }
    } else {
        fprintf(stderr, "%s<vals>: EMPTY\n", indent.base);
    }

    zst_string_free(&indent);
}

ZST_DEF void zst_flag_init(zst_flag_t *flag)
{
    if (!flag) return;

    flag->isused = false;
    flag->type = FLAG_NO_ARG;
    flag->name = (zst_string_t) {0};
    flag->info = (zst_string_t) {0};
    zst_dyna_init(&flag->vals, sizeof(zst_string_t), zst_string_free);
}

ZST_DEF void zst_cmdline_init(zst_cmdline_t *cmdl)
{
    cmdl->prog = (zst_string_t) {0};
    zst_dyna_init(&cmdl->flags, sizeof(zst_flag_t), zst_flag_free);
}

ZST_DEF void zst_cmdline_define_flag(zst_cmdline_t *cmdl, zst_flag_type_t type, const char *name, const char *info)
{
    if (type < 0 || type > NR_FLAG || !name) return;

    zst_flag_t flag = {0};
    zst_flag_init(&flag);

    flag.type = type;
    zst_string_append(&flag.name, name);
    zst_string_append(&flag.info, info);

    zst_dyna_append(&cmdl->flags, &flag);
}

static inline char *skip_dash_(char *arg)
{
    for (size_t i = 0; i < strlen(arg); i++) {
        if (arg[i] == '-') continue;
        return arg + i;
    }

    return NULL;
}

ZST_DEF zst_flag_t *zst_cmdline_get_flag(zst_cmdline_t *cmdl, const char *flagname)
{
    for (size_t i = 0; i < cmdl->flags.count; i++) {
        zst_flag_t *flag = zst_dyna_get(&cmdl->flags, i);
        if (strcmp(flagname, flag->name.base) == 0) return flag;
    }

    return NULL;
}

static inline int skip_values_(int pos, char **argv)
{
    while (argv[pos]) {
        if (argv[pos][0] == '-') break;
        pos++;
    }
    return pos;
}

ZST_DEF void zst_cmdline_parse(zst_cmdline_t *cmdl, int argc, char **argv)
{
    int pos = 0; // track the arg

    zst_string_append(&cmdl->prog, argv[pos++]);

    pos = skip_values_(pos, argv);

    while (pos < argc) {
        const char *p1 = skip_dash_(argv[pos]);
        if (!p1) {
            pos = skip_values_(pos + 1, argv);
            continue;
        }
        const char *p2 = strchr(p1, '=');
        bool has_equal = (p2 != NULL);

        zst_string_t flagname = {0};
        if (has_equal) {
            flagname = zst_string_sub(p1, 0, p2 - p1);
        } else {
            zst_string_append(&flagname, p1);
        }
        zst_flag_t *flag = zst_cmdline_get_flag(cmdl, flagname.base);
        zst_string_free(&flagname);
        if (!flag) {
            pos = skip_values_(pos + 1, argv);
            continue;
        }

        flag->isused = true;

        /* add target values for this flag according to its type */
        if (has_equal) {
            if (flag->type != FLAG_SINGLE_ARG) {
                zst_log(LOG_FATAL, "flag <%s> can only receive one value", flag->name);
            }

            zst_string_t val = {0};
            if (*(p2 + 1)) {
                zst_string_append(&val, p2 + 1);
                zst_dyna_append(&flag->vals, &val);
            }

            pos = skip_values_(pos + 1, argv);
        } else { 
            pos++; // skip the current flag

            zst_string_t val;
            switch (flag->type) {
            case FLAG_NO_ARG:
                pos = skip_values_(pos, argv);
                break;

            case FLAG_SINGLE_ARG:
                if (flag->vals.count == 1) break;
                if (pos >= argc) {
                    zst_log(LOG_FATAL, "flag <%s> should receive one value", flag->name.base);
                }
                if (argv[pos][0] == '-') {
                    zst_log(LOG_FATAL, "'%s' should be an argument of flag <%s>", argv[pos], flag->name.base);
                }
                val = (zst_string_t) {0};
                zst_string_append(&val, argv[pos]);
                zst_dyna_append(&flag->vals, &val);
                pos = skip_values_(pos, argv);
                break;

            case FLAG_MULTI_ARG:
                while (pos < argc) {
                    if (argv[pos][0] == '-') break;
                    val = (zst_string_t) {0};
                    zst_string_append(&val, argv[pos]);
                    zst_dyna_append(&flag->vals, &val);
                    pos++;
                }
                break;

            default:
                zst_log(LOG_FATAL, "invalid flag type");
                break;
            }
        }
    }
}

ZST_DEF void zst_cmdline_usage(zst_cmdline_t *cmdl)
{
    fprintf(stderr, "Usage: %s ...\n", cmdl->prog.base);
    for (size_t i = 0; i < cmdl->flags.count; i++) {
        zst_flag_t *flag = zst_dyna_get(&cmdl->flags, i);
        fprintf(stderr, "  %-10s %s\n", flag->name.base, flag->info.base);
    }
}

ZST_DEF bool zst_cmdline_isuse(zst_cmdline_t *cmdl, const char *flagname)
{
    if (!flagname) return false;

    zst_flag_t *flag = zst_cmdline_get_flag(cmdl, flagname);
    if (!flag) return false;
    return flag->isused;
}

ZST_DEF void zst_cmdline_free(void *arg)
{
    zst_cmdline_t *cmdl = (zst_cmdline_t *) arg;
    zst_string_free(&cmdl->prog);
    zst_dyna_free(&cmdl->flags);
}

ZST_DEF void zst_flag_free(void *arg)
{
    zst_flag_t *flag = (zst_flag_t *) arg;
    flag->isused = false;
    flag->type = FLAG_NO_ARG;
    zst_string_free(&flag->name);
    zst_dyna_free(&flag->vals);
    zst_string_free(&flag->info);
}

/* MODULE: ZST_FILE_SYSTEM */

static inline int count_line(char *buf, size_t size)
{
    if (!buf) return 0;

    int count = 0;
    for (size_t i = 0; i < size; i++) {
        if (buf[i] == '\n') count++;
    }

    if (size > 0 && buf[size - 1] != '\n') count++;

    return count;
}

ZST_DEF unsigned long long zst_fs_get_timestamp(const char *pathname)
{
    unsigned long long timestamp = 0;
    if (zst_fs_typeof(pathname) == FT_NOET) return 0;

#ifdef _WIN32
    HANDLE hFile = CreateFile(pathname, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        zst_log(LOG_FATAL, "failed to open file: %s", pathname);
    }

    FILETIME ftCreate, ftAccess, ftWrite;
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
        SYSTEMTIME stUTC, stLocal;
        FileTimeToSystemTime(&ftWrite, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        
        ULONGLONG ft = (((ULONGLONG) ftWrite.dwHighDateTime) << 32) + ftWrite.dwLowDateTime;
        timestamp = (unsigned long long)((ft - 116444736000000000LL) / 10000000);
    } else {
        zst_log(LOG_FATAL, "failed to get file time: %s", pathname);
    }
    CloseHandle(hFile);
#else
    struct stat sb;
    if (stat(pathname, &sb) == -1) {
        zst_log(LOG_FATAL, "failed to get file stats: %s", pathname);
    }

    timestamp = (unsigned long long) sb.st_mtime;
#endif // platform

    return timestamp;
}

ZST_DEF char *zst_fs_get_name(const char *pathname)
{
    if (!pathname) return NULL;

#ifdef _WIN32
    char delim = '\\';
#else
    char delim = '/';
#endif // platform

    size_t len = strlen(pathname);
    char buf[len + 1];
    snprintf(buf, sizeof(buf), "%s", pathname);

    if (buf[len - 1] == delim) buf[len - 1] = '\0';

    char *pos = strrchr(buf, delim);
    if (pos) return (char *) pathname + (pos - buf + 1);
    return (char *) pathname;
}

ZST_DEF bool zst_fs_pwd(char *buf, size_t buf_size)
{
    if (!buf || buf_size == 0) return false;

#ifdef _WIN32
    if (GetCurrentDirectory(buf_size, buf)) return true;
    else return false;
#else
    if (getcwd(buf, buf_size)) return true;
    else return false;
#endif // platform 
}

ZST_DEF bool zst_fs_cd(const char *pathname)
{
    if (!pathname) return false;

#ifdef _WIN32
    if (SetCurrentDirectory(pathname)) return true;
    else return false;
#else
    if (chdir(pathname) == 0) return true;
    else return false;
#endif // platform
}

ZST_DEF bool zst_fs_move(const char *src, const char *dest)
{
    if (zst_fs_copy(src, dest) && zst_fs_remove(src)) {
        return true;
    }
    return false;
}

ZST_DEF bool zst_fs_remove_all(zst_dyna_t *files)
{
    bool status = true;
    for (size_t i = 0; i < files->count; i++) {
        zst_string_t *file = zst_dyna_get(files, i);
        status &= zst_fs_remove(file->base);
    }
    return status;
}

static bool copy_file_(const char *src, const char *dest)
{
    char dest_path[FS_MAX_PATH_SIZE];
    if (zst_fs_typeof(dest) == FT_NOET) {
        if (dest[strlen(dest) - 1] == '\\' || dest[strlen(dest) - 1] == '/')
            zst_fs_mkdir(dest);
    }

    if (zst_fs_typeof(dest) == FT_DIR) {
        snprintf(dest_path, sizeof(dest_path), "%s/%s",
                dest, zst_fs_get_name(src));
    } else {
        snprintf(dest_path, sizeof(dest_path), "%s", dest);
    }

    zst_meta_file_t mf = {0};
    if (!zst_fs_load_file(src, &mf)) return false;
    if (!zst_fs_dump_file(dest_path, mf.content, mf.size)) {
        zst_fs_free_meta_file(&mf);
        return false;
    }
    zst_fs_free_meta_file(&mf);
    return true;
}

static bool copy_dir_recursively_(const char *src, const char *dest)
{
#define return_defer(code) do { status = code; goto defer; } while (0)

    bool status = true;

    if (!zst_fs_remove(dest)) {
        zst_log(LOG_ERROR, "could not remove the directory '%s'", dest);
        return false;
    }
    if (!zst_fs_mkdir(dest)) {
        zst_log(LOG_ERROR, "could not create the directory '%s'", dest);
        return false;
    }

    char dest_path[FS_MAX_PATH_SIZE + 1];
    size_t len = strlen(dest);

    zst_meta_dir_t md = {0};
    zst_fs_load_dir(src, &md);

    for (size_t i = 0; i < md.f_cnt; i++) {
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, zst_fs_get_name(md.files[i]));
        if (!copy_file_(md.files[i], dest_path)) return_defer(false);
        dest_path[len] = '\0';
    }

    for (size_t i = 0; i < md.d_cnt; i++) {
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, zst_fs_get_name(md.dirs[i]));
        if (!copy_dir_recursively_(md.dirs[i], dest_path)) return_defer(false);
        dest_path[len] = '\0';
    }

defer:
    zst_fs_free_meta_dir(&md);
    return status;

#undef return_defer
}

ZST_DEF bool zst_fs_copy(const char *src, const char *dest)
{
    int src_type = zst_fs_typeof(src);
    int dest_type = zst_fs_typeof(dest);

    if (src_type == FT_NOET) {
        zst_log(LOG_ERROR, "source file to be copied is not exist");
        return false;
    }
    if (src_type == FT_DIR && dest_type == FT_REG) {
        zst_log(LOG_ERROR, "not compatible file type to copy");
        return false;
    }

    if (src_type == FT_DIR) return copy_dir_recursively_(src, dest);
    else return copy_file_(src, dest);
}

ZST_DEF bool zst_fs_mkdir(const char *pathname)
{
    char path_copy[FS_MAX_PATH_SIZE];
    strncpy(path_copy, pathname, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    /* process step by step in the path string */
    char *dir = path_copy;

#ifdef _WIN32
    while ((dir = strchr(dir, '\\')) != NULL) {
        *dir = '\0';
        if (path_copy[0] != '\0' && _mkdir(path_copy) != 0 && errno != EEXIST) {
            return false;
        }
        *dir = '\\';
        dir++;
    }
    return (_mkdir(pathname) == 0 || errno == EEXIST);
#else
    while ((dir = strchr(dir, '/')) != NULL) {
        *dir = '\0';
        if (path_copy[0] != '\0' && // avoid path_copy to be "" in case /home/... 
                mkdir(path_copy, S_IRWXU | S_IRWXG | S_IRWXO) != 0 &&
                errno != EEXIST) {
            return false;
        }
        /* restore the seperator and process next path */
        *dir = '/';
        dir++;
    }

    /* create final directory */
    return (mkdir(pathname, S_IRWXU | S_IRWXG | S_IRWXO) == 0 ||
            errno == EEXIST);
#endif // platform
}

ZST_DEF bool zst_fs_touch(const char *pathname)
{
    FILE *fp = fopen(pathname, "a");
    if (fp == NULL) return false;
    fclose(fp);
    return true;
}

static bool remove_dir_recursively_(const char *dirpath)
{
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char dir[FS_MAX_PATH_SIZE];
    snprintf(dir, sizeof(dir), "%s\\*", dirpath);

    hFind = FindFirstFile(dir, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) return false;

    do {
        if (strcmp(findFileData.cFileName, ".") != 0 &&
                strcmp(findFileData.cFileName, "..") != 0) {
            char filepath[FS_MAX_PATH_SIZE];
            snprintf(filepath, sizeof(filepath), "%s\\%s",
                    dirpath, findFileData.cFileName);

            int type = zst_fs_typeof(filepath);
            if (type == FT_NOET) break;
            if (type == FT_DIR) remove_dir_recursively_(filepath);
            else DeleteFile(filepath); 
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return RemoveDirectory(dirpath) != 0;
#else
    DIR *dp = opendir(dirpath);
    if (!dp) return false;

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 &&
                strcmp(entry->d_name, "..") != 0) {
            char filepath[FS_MAX_PATH_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s",
                    dirpath, entry->d_name);

            int type = zst_fs_typeof(filepath);
            if (type == FT_NOET) break;
            if (type == FT_DIR) {
                if (!remove_dir_recursively_(filepath)) {
                    closedir(dp);
                    return false;
                }
            } else {
                if (unlink(filepath) != 0) {
                    closedir(dp);
                    return false;
                }
            }
        }
    }
    closedir(dp);
    return rmdir(dirpath) == 0;
#endif // platform
}

ZST_DEF zst_dyna_t zst_fs_match_recursively(const char *pathname, const char *pattern)
{
    zst_dyna_t res = {0};
    if (!pathname || !pattern) return res;

    res = zst_fs_match(pathname, pattern);

    zst_meta_dir_t md = {0};
    if (!zst_fs_load_dir(pathname, &md)) goto defer;

    for (size_t i = 0; i < md.d_cnt; i++) {
        zst_dyna_t sub_res = zst_fs_match_recursively(md.dirs[i], pattern);
        zst_dyna_merge(&res, &sub_res);
    }

defer:
    zst_fs_free_meta_dir(&md);
    return res;
}

ZST_DEF zst_dyna_t zst_fs_find_recursively(const char *pathname, int attrs)
{
    zst_dyna_t res = {0};
    if (!pathname || !attrs) return res;

    res = zst_fs_find(pathname, attrs);

    zst_meta_dir_t md = {0};
    if (!zst_fs_load_dir(pathname, &md)) goto defer;

    for (size_t i = 0; i < md.d_cnt; i++) {
        zst_dyna_t sub_res = zst_fs_find_recursively(md.dirs[i], attrs);
        zst_dyna_merge(&res, &sub_res);
    }

defer:
    zst_fs_free_meta_dir(&md);
    return res;
}

ZST_DEF zst_dyna_t zst_fs_match(const char *pathname, const char *pattern)
{
    zst_dyna_t res = {0};
    zst_dyna_init(&res, sizeof(zst_string_t), zst_string_free);

    if (!pathname || !pattern) return res;

    int type = zst_fs_typeof(pathname);
    if (type == FT_DIR) {
        zst_meta_dir_t md = {0};
        zst_fs_load_dir(pathname, &md);
        for (size_t i = 0; i < md.f_cnt; i++) {
            if (zst_wildcard_match(md.files[i], pattern)) {
                zst_string_t file = {0};
                zst_string_append(&file, md.files[i]); 
                zst_dyna_append(&res, &file);
            }
        }
        zst_fs_free_meta_dir(&md);
    } else if (type == FT_REG) {
        if (zst_wildcard_match(pathname, pattern)) {
            zst_string_t file = {0};
            zst_string_append(&file, pathname);
            zst_dyna_append(&res, &file);
        }
    } 

    return res;
}

ZST_DEF zst_dyna_t zst_fs_find(const char *pathname, int attrs)
{
    zst_dyna_t res = {0};
    zst_dyna_init(&res, sizeof(zst_string_t), zst_string_free);

    if (!pathname) return res;

    int type = zst_fs_typeof(pathname);
    if (type == FT_DIR) {
        zst_meta_dir_t md = {0};
        zst_fs_load_dir(pathname, &md);
        for (size_t i = 0; i < md.f_cnt; i++) {
            if (zst_fs_check_perm(md.files[i], attrs)) {
                zst_string_t file = {0};
                zst_string_append(&file, md.files[i]); 
                zst_dyna_append(&res, &file);
            }
        }
        zst_fs_free_meta_dir(&md);
    } else if (type == FT_REG) {
        if (zst_fs_check_perm(pathname, attrs)) {
            zst_string_t file = {0};
            zst_string_append(&file, pathname);
            zst_dyna_append(&res, &file);
        }
    } 

    return res;
}

ZST_DEF bool zst_fs_remove(const char *pathname)
{
    int type = zst_fs_typeof(pathname);
    if (type == FT_NOET) return true;
    else if (type == FT_DIR) return remove_dir_recursively_(pathname);
    else return remove(pathname) == 0;
}

ZST_DEF int zst_fs_typeof(const char *pathname)
{
    if (!pathname) return FT_NOET;

#ifdef _WIN32
    DWORD attr = GetFileAttributesA(pathname);
    if (attr == INVALID_FILE_ATTRIBUTES) return FT_NOET;

    if (attr & FILE_ATTRIBUTE_DIRECTORY) return FT_DIR;
    else return FT_REG;
#else
    struct stat sb;
    if (stat(pathname, &sb) == -1) return FT_NOET;

    switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:
    case S_IFCHR:
    case S_IFIFO:
    case S_IFLNK:
    case S_IFSOCK:
    case S_IFREG:
        return FT_REG;
    case S_IFDIR:
        return FT_DIR;
    default: 
        return FT_NOET;
    }
#endif // platform
}

ZST_DEF bool zst_fs_load_dir(const char *dirname, zst_meta_dir_t *res)
{
    if (!dirname) return false;

    if (zst_fs_typeof(dirname) != FT_DIR) return false;

    res->name = strdup(dirname);
    assert(res->name != NULL);

#ifdef _WIN32
    bool has_slash = dirname[strlen(dirname) - 1] == '\\';

    char buf[FS_MAX_PATH_SIZE];
    if (!has_slash) snprintf(buf, sizeof(buf), "%s\\*", dirname);
    else snprintf(buf, sizeof(buf), "%s*", dirname);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(buf, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        free(res->name);
        res->name = NULL;
        return false;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 ||
                strcmp(findFileData.cFileName, "..") == 0)
            continue;

        size_t len = strlen(dirname) + strlen(findFileData.cFileName) + 2;
        char *full_path = (char *) malloc(len);
        if (!has_slash) snprintf(full_path, len, "%s\\%s", dirname, findFileData.cFileName);
        else snprintf(full_path, len, "%s%s", dirname, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            res->dirs = realloc(res->dirs, sizeof(char *) * (res->d_cnt + 1));
            assert(res->dirs != NULL);
            res->dirs[res->d_cnt] = strdup(full_path);
            assert(res->dirs[res->d_cnt] != NULL);
            res->d_cnt++;
        } else {
            res->files = realloc(res->files, sizeof(char *) * (res->f_cnt + 1));
            assert(res->files != NULL);
            res->files[res->f_cnt] = strdup(full_path);
            assert(res->files[res->f_cnt] != NULL);
            res->f_cnt++;
        }

        free(full_path);
        res->count++;
    } while (FindNextFile(hFind, &findFileData));

    FindClose(hFind);
    return true;
#else
    bool has_slash = dirname[strlen(dirname) - 1] == '/';

    DIR *dp = opendir(dirname);
    if (!dp) {
        free(res->name);
        res->name = NULL;
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t len = strlen(dirname) + strlen(entry->d_name) + 2;
        char *full_path = (char *) malloc(len);
        if (!has_slash) snprintf(full_path, len, "%s/%s", dirname, entry->d_name);
        else snprintf(full_path, len, "%s%s", dirname, entry->d_name);

        int type = zst_fs_typeof(full_path);
        if (type == FT_REG) {
            res->files = realloc(res->files, sizeof(char *) * (res->f_cnt + 1));
            assert(res->files != NULL);
            res->files[res->f_cnt] = strdup(full_path);
            assert(res->files[res->f_cnt] != NULL);
            res->f_cnt++;
        } else {
            res->dirs = realloc(res->dirs, sizeof(char *) * (res->d_cnt + 1));
            assert(res->dirs != NULL);
            res->dirs[res->d_cnt] = strdup(full_path);
            assert(res->dirs[res->d_cnt] != NULL);
            res->d_cnt++;
        }

        free(full_path);
        res->count++;
    }

    closedir(dp);
    return true;
#endif // platform
}

ZST_DEF bool zst_fs_load_file(const char *filename, zst_meta_file_t *res)
{
#define return_defer(code) do { status = code; goto defer; } while (0) 

    int status = 0;

    if (!filename) return false;

    res->type = zst_fs_typeof(filename);
    if (res->type != FT_REG) return false;

    res->name = strdup(filename);
    assert(res->name != NULL);

    zst_fd_t fd = zst_fs_open_to_read(filename);
    if (fd == ZST_INVALID_FD) return_defer(-1);

    res->size = zst_fs_get_size(filename);
    if (res->size == 0) return_defer(-1);

    res->content = malloc(res->size + 1);    
    assert(res->content != NULL);

    size_t read_size = zst_fs_read(fd, res->content, res->size);
    if (read_size != res->size) return_defer(-1);

    res->content[res->size] = '\0';
    res->line = count_line(res->content, res->size);
    res->attr = zst_fs_get_attr(filename);

defer:
    if (status == -1) zst_fs_free_meta_file(res);
    if (fd != ZST_INVALID_FD) zst_fs_close(fd);
    return status == 0;

#undef return_defer
}

ZST_DEF void zst_fs_free_meta_file(zst_meta_file_t *mf)
{
    if (mf->name) free(mf->name);
    if (mf->content) free(mf->content);

    mf->name = NULL;
    mf->content = NULL;
    mf->type = FT_NOET;
    mf->size = 0;
    mf->line = 0;
}

ZST_DEF size_t zst_fs_get_size(const char *filename)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &fileInfo) == 0) {
        zst_log(LOG_ERROR, "failed to get file size");
        return 0;
    }
    LARGE_INTEGER fileSize;
    fileSize.LowPart = fileInfo.nFileSizeLow;
    fileSize.HighPart = fileInfo.nFileSizeHigh;
    return (size_t) fileSize.QuadPart;
#else
    struct stat st;
    if (stat(filename, &st) == -1) {
        zst_log(LOG_ERROR, "failed to get file size");
        return 0;
    }
    return (size_t) st.st_size;
#endif // platform
}

ZST_DEF int zst_fs_get_attr(const char *filename)
{
    int perm = 0;

#ifdef _WIN32
    if (_access(filename, 4) == 0) perm |= FA_READ;
    if (_access(filename, 2) == 0) perm |= FA_WRITE;
    const char *ext = strrchr(filename, '.');
    if (ext) { 
        if (strcmp(ext, ".exe") == 0 || strcmp(ext, ".bat") == 0 ||
            strcmp(ext, ".cmd") == 0 || strcmp(ext, ".com") == 0) {
            perm |= FA_EXEC;
        }
    }
#else
    struct stat sb;
    if (stat(filename, &sb) != 0) {
        return perm;
    }

    if (sb.st_mode & S_IRUSR) perm |= FA_READ;
    if (sb.st_mode & S_IWUSR) perm |= FA_WRITE;
    if (sb.st_mode & S_IXUSR) perm |= FA_EXEC;
#endif // platform

    return perm;
}

ZST_DEF bool zst_fs_check_perm(const char *filename, int perm)
{
    return zst_fs_get_attr(filename) & perm;
}

ZST_DEF void zst_fs_free_meta_dir(zst_meta_dir_t *md)
{
    if (md->name) free(md->name);
    if (md->files) {
        for (size_t i = 0; i < md->f_cnt; i++) {
            if (md->files[i])
                free(md->files[i]);
            md->files[i] = NULL;
        }
        free(md->files);
    }
    if (md->dirs) {
        for (size_t i = 0; i < md->d_cnt; i++) {
            if (md->dirs[i])
                free(md->dirs[i]);
            md->dirs[i] = NULL;
        }
        free(md->dirs);
    }

    md->name = NULL;
    md->files = NULL;
    md->dirs = NULL;
    md->f_cnt = 0;
    md->d_cnt = 0;
    md->count = 0;
}

ZST_DEF bool zst_fs_dump_file(const char *dest_file, const char *src_buf, size_t size)
{
#define return_defer(code) do { status = code; goto defer; } while (0)

    bool status = true;

    zst_fd_t fd = zst_fs_open_to_write(dest_file);
    if (fd == ZST_INVALID_FD) return false;

    size_t write_size = zst_fs_write(fd, src_buf, size);
    if (write_size != size) return_defer(false);

defer:
    zst_fs_close(fd);
    return status;

#undef return_defer
}

/* MODULE: ZST_DS_DYNAMIC_ARRAY */

ZST_DEF void zst_dyna_merge(zst_dyna_t *to, zst_dyna_t *from)
{
    if (to->size != from->size) return;

    from->free_item = NULL;

    for (size_t i = 0; i < from->count; i++) {
        void *elem = zst_dyna_get(from, i);
        zst_dyna_append(to, elem);
    }

    zst_dyna_free(from);
}

ZST_DEF void zst_dyna_init(zst_dyna_t *da, size_t size, void (*free_item)(void *))
{
    da->base = NULL;
    da->count = 0;
    da->capacity = 0;
    da->free_item = free_item;
    da->size = size;
}

ZST_DEF void zst_dyna_append(zst_dyna_t *da, void *elem)
{
    if (da->capacity <= da->count) {
        da->capacity = (da->capacity == 0) ? 10 : (2 * da->capacity);
        da->base = realloc(da->base, da->size * da->capacity);
        assert(da->base != NULL);
    }
    void *dest = zst_dyna_get(da, da->count++);
    memcpy(dest, elem, da->size);
}

ZST_DEF void zst_dyna_insert(zst_dyna_t *da, size_t idx, void *elem)
{
    if (idx > da->count) {
        return;
    } else if (idx == da->count) {
        zst_dyna_append(da, elem);
    } else {
        if (da->capacity <= da->count) {
            da->capacity = (da->capacity == 0) ? 10 : (2 * da->capacity);
            da->base = realloc(da->base, da->size * da->capacity);
            assert(da->base != NULL);
        }
        void *dest = zst_dyna_get(da, idx);
        memcpy((char *) dest + da->size, dest, (da->count-idx) * da->size);
        memcpy(dest, elem, da->size);
        da->count++;
    }
}

ZST_DEF void zst_dyna_remove(zst_dyna_t *da, size_t idx)
{
    if (da->count > 0 && idx < da->count) {
        void *elem = zst_dyna_get(da, idx);
        if (da->free_item) da->free_item(elem);
        memcpy(zst_dyna_get(da, idx), zst_dyna_get(da, idx + 1),
                da->size * (da->count - idx - 1));
        da->count--;
    }
}

/* zst_dyna_set means: { free_item(da[idx]); da[idx] = elem; } */
ZST_DEF void zst_dyna_set(zst_dyna_t *da, size_t idx, void *elem)
{
    if (idx >= da->count) return;
    void *dest = zst_dyna_get(da, idx);
    if (da->free_item && dest) da->free_item(dest);
    memcpy(dest, elem, da->size);
}

/* zst_dyna_get means: return da[idx]; */
ZST_DEF void *zst_dyna_get(zst_dyna_t *da, size_t idx)
{
    if (idx >= da->count) return NULL;
    return (char *) da->base + da->size * idx;
}

ZST_DEF void zst_dyna_free(zst_dyna_t *da)
{
    if (da->free_item) {
        for (size_t i = 0; i < da->count; i++) {
            void *item = (char *) da->base + da->size * i;
            da->free_item(item);
        }
    }
    free(da->base);
    da->count = 0;
    da->capacity = 0;
    da->free_item = NULL;
    da->size = 0;
}

/* MODULE: ZST_DS_STRING */

ZST_DEF void zst_string_append(zst_string_t *str, const char *fmt, ...)
{
    if (!fmt) return;

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    size_t len = vsnprintf(NULL, 0, fmt, args_copy);
    if (len <= 0) {
        va_end(args_copy);
        va_end(args);
        return;
    }
    va_end(args_copy);

    char new_str[len+1];
    vsnprintf(new_str, len + 1, fmt, args);
    new_str[len] = '\0';

    while (str->capacity <= str->length + len + 1) {
        str->capacity = (str->capacity == 0) ? 128 : (2 * str->capacity);
        str->base = realloc(str->base, str->capacity);
        assert(str->base != NULL);
    }
    memcpy(str->base + str->length, new_str, len);
    str->length += len;
    str->base[str->length] = '\0';

    va_end(args);
}

ZST_DEF zst_string_t zst_string_replace(const char *str, const char *s_old, const char *s_new)
{
    zst_string_t res = {0};
    char *pos = strstr(str, s_old);

    if (!pos) return res;
    size_t len_before = pos - str;

    zst_string_t tmp = {0};
    zst_string_append(&tmp, str);
    tmp.base[len_before] = '\0';

    zst_string_append(&res, tmp.base); 
    zst_string_append(&res, s_new);
    zst_string_append(&res, pos + strlen(s_old));

    zst_string_free(&tmp);
    return res; 
}

ZST_DEF zst_string_t zst_string_repeat(const char *str, size_t times)
{
    zst_string_t res = {0};
    while (times--) zst_string_append(&res, str);
    return res;
}

/* [src, dest) */
ZST_DEF zst_string_t zst_string_sub(const char *str, size_t src, size_t dest)
{
    zst_string_t res = {0};
    if (src >= strlen(str) || dest > strlen(str) || src >= dest) return res;

    char buf[dest - src + 1];
    memcpy(buf, str + src, dest - src);
    buf[dest - src] = '\0';
    zst_string_append(&res, buf);

    return res;
}

static zst_string_t zst_string_concat_(const char *delim, ...)
{
    va_list args;
    va_start(args, delim);

    char *p;
    zst_string_t res = {0};
    size_t pos = 0;

    while ((p = va_arg(args, char *)) != NULL) {
        pos = res.length + strlen(p);
        zst_string_append(&res, "%s%s", p, delim);
    }
    va_end(args);

    res.base[pos] = '\0';
    res.length = pos;

    return res;
}

ZST_DEF void zst_string_free(void *arg)
{
    zst_string_t *str = (zst_string_t *) arg;
    if (str->base != NULL) free(str->base);
    str->base = NULL;
    str->length = 0;
    str->capacity = 0;
}

/* MODULE: ZST_DS_STACK */

ZST_DEF void zst_stack_init(zst_stack_t *stk, size_t size, void (*free_item)(void *))
{
    stk->base = NULL;
    stk->top = -1;
    stk->capacity = 0;
    stk->free_item = free_item;
    stk->size = size;
    if (size != 0) zst_dyna_init(&stk->gc, size, free_item);
}

ZST_DEF void zst_stack_push(zst_stack_t *stk, void *elem)
{
    stk->top++;
    if (stk->capacity <= (size_t) stk->top) {
        stk->capacity = (stk->capacity == 0) ? 10 : (2 * stk->capacity);
        stk->base = realloc(stk->base, stk->size * stk->capacity);
        assert(stk->base != NULL);
    }
    void *dest = (char *) stk->base + stk->size * stk->top;
    memcpy(dest, elem, stk->size);
}

ZST_DEF void *zst_stack_pop(zst_stack_t *stk)
{
    if (stk->top == -1) return NULL; 
    void *item = (char *) stk->base + stk->size * stk->top--;
    zst_dyna_append(&stk->gc, item);
    return item;
}

ZST_DEF void *zst_stack_top(zst_stack_t *stk)
{
    if (stk->top == -1) return NULL; 
    return (char *) stk->base + stk->size * stk->top;
}

ZST_DEF void zst_stack_free(zst_stack_t *stk)
{
    if (stk->free_item) {
        for (int i = 0; i <= stk->top; i++) {
            void *item = (char *) stk->base + stk->size * i;
            stk->free_item(item);
        }
    }
    zst_dyna_free(&stk->gc);

    free(stk->base);
    stk->top = -1;
    stk->capacity = 0;
    stk->free_item = NULL;
    stk->size = 0;
}

/* MODULE: ZST_DS_QUEUE */

static void *zst_queue_create_node(void *elem, size_t size)
{
    zst_queue_node_t *node = malloc(sizeof(zst_queue_node_t));
    assert(node != NULL);

    node->data = malloc(size);
    assert(node->data != NULL);
    memcpy(node->data, elem, size);
    node->next = NULL;

    return node;
}

ZST_DEF void zst_queue_init(zst_queue_t *queue, size_t size, void (*free_item)(void *))
{
    queue->head = (zst_queue_node_t) {0};
    queue->front = NULL;
    queue->rear  = NULL;
    queue->count = 0;
    queue->size  = size;
    queue->free_item = free_item;
    if (size != 0) zst_dyna_init(&queue->gc, sizeof(void *), NULL);
}

ZST_DEF void zst_queue_free(zst_queue_t *queue)
{
    zst_queue_node_t *node = queue->head.next, *tmp = NULL;
    while (node) {
        tmp = node->next;
        if (queue->free_item) 
            queue->free_item(node->data);
        free(node->data);
        free(node);
        node = tmp;
    }

    for (size_t i = 0; i < queue->gc.count; i++) {
        void **gc_item = zst_dyna_get(&queue->gc, i);
        if (queue->free_item) queue->free_item(*gc_item);
        free(*gc_item);
    }
    zst_dyna_free(&queue->gc);

    zst_queue_init(queue, 0, NULL);
}

ZST_DEF void zst_queue_push(zst_queue_t *queue, void *elem)
{
    zst_queue_node_t *node = zst_queue_create_node(elem, queue->size);

    if (zst_queue_isempty(queue)) {
        queue->front = node;
        queue->rear = node;
        queue->head.next = queue->front;
    } else {
        queue->rear->next = node;
        queue->rear = queue->rear->next;
    }

    queue->count++;
}

ZST_DEF void *zst_queue_pop(zst_queue_t *queue)
{
    if (zst_queue_isempty(queue)) {
        return NULL;
    } else if (queue->front == queue->rear) {
        /* just one element */
        void *res = queue->front->data;

        zst_dyna_append(&queue->gc, &queue->front->data);
        free(queue->front);
        queue->head = (zst_queue_node_t) {0};
        queue->front = NULL;
        queue->rear  = NULL;
        queue->count = 0;

        return res;
    } else {
        void *res = queue->front->data;
        queue->head.next = queue->front->next;

        zst_dyna_append(&queue->gc, &queue->front->data);
        free(queue->front);

        queue->front = queue->head.next;
        queue->count--;

        return res;
    }
}

ZST_DEF void *zst_queue_front(zst_queue_t *queue)
{
    if (zst_queue_isempty(queue)) return NULL;
    return queue->front->data;
}

ZST_DEF void *zst_queue_rear(zst_queue_t *queue)
{
    if (zst_queue_isempty(queue)) return NULL;
    return queue->rear->data;
}

/* MODULE: ZST_LOG */

#define ZST_LOG_COLOR_RESET      "\x1b[0m"
#define ZST_LOG_COLOR_FG_RED     "\x1b[31m"
#define ZST_LOG_COLOR_FG_GREEN   "\x1b[32m"
#define ZST_LOG_COLOR_FG_YELLOW  "\x1b[33m"
#define ZST_LOG_COLOR_BG_RED     "\x1b[41m"
#define ZST_LOG_COLOR_BG_GREEN   "\x1b[42m"

ZST_DEF void zst_log(int type, const char *fmt, ...)
{
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    size_t len = vsnprintf(NULL, 0, fmt, args_copy);
    if (len <= 0) {
        va_end(args_copy);
        va_end(args);
        return;
    }
    va_end(args_copy);

    size_t buf_size = len + 100;
    char buf1[buf_size], buf2[buf_size];
    bool is_exit = false;

    switch ((type)) {
    case LOG_INFO:
        snprintf(buf1, buf_size, "[%sINFO%s] %s\n", ZST_LOG_COLOR_FG_YELLOW,
                ZST_LOG_COLOR_RESET, fmt);
        break;

    case LOG_ERROR:
        snprintf(buf1, buf_size, "[%sERROR%s] %s\n", ZST_LOG_COLOR_FG_RED,
                ZST_LOG_COLOR_RESET, fmt);
        break;

    case LOG_GOOD:
        snprintf(buf1, buf_size, "[%sGOOD%s] %s\n", ZST_LOG_COLOR_FG_GREEN,
                ZST_LOG_COLOR_RESET, fmt);
        break;

    case LOG_FATAL:
        snprintf(buf1, buf_size, "[%sFATAL%s] %s\n", ZST_LOG_COLOR_BG_RED,
                ZST_LOG_COLOR_RESET, fmt);
        is_exit = true;
        break;

    case LOG_TODO:
        snprintf(buf1, buf_size, "[%sTODO%s] %s\n", ZST_LOG_COLOR_BG_GREEN,
                ZST_LOG_COLOR_RESET, fmt);
        is_exit = true;
        break;

    default:
        break;
    }

    vsnprintf(buf2, buf_size, buf1, args);
    fprintf(stderr, "%s", buf2);

    va_end(args);

    if (is_exit) exit(EXIT_FAILURE);
}

/* MODULE: ZST_FORGER */

static void zst_cmd_append_arg_(zst_cmd_t *cmd, ...)
{
    va_list ap;
    va_start(ap, cmd);

    char *arg = NULL;
    while ((arg = va_arg(ap, char *)) != NULL) {
        zst_string_t str = {0};
        zst_string_append(&str, arg);
        zst_dyna_append(&cmd->args, &str);
        cmd->count++;
    }

    va_end(ap);
}

ZST_DEF void zst_cmd_dump(zst_cmd_t *cmd)
{
    zst_string_t cmd_string = {0};

    for (size_t i = 0; i < cmd->args.count; i++) {
        zst_string_t *arg = zst_dyna_get(&cmd->args, i);
        zst_string_append(&cmd_string, " %s", arg->base);
    }

    zst_log(LOG_INFO, cmd_string.base);

    zst_string_free(&cmd_string);
}

ZST_DEF void zst_cmd_init(zst_cmd_t *cmd)
{
    zst_dyna_init(&cmd->args, sizeof(zst_string_t), zst_string_free);
    cmd->count = 0;
}

ZST_DEF void zst_cmd_free(zst_cmd_t *cmd)
{
    zst_dyna_free(&cmd->args);
    cmd->count = 0;
}

ZST_DEF int zst_cmd_run(zst_cmd_t *cmd)
{
    zst_string_t cmd_string = {0};

    for (size_t i = 0; i < cmd->count; i++) {
        char *arg = ((zst_string_t *) zst_dyna_get(&cmd->args, i))->base;
        zst_string_append(&cmd_string, " %s", arg);
    }
#ifdef _WIN32
    zst_string_append(&cmd_string, " %s", "-D__USE_MINGW_ANSI_STDIO");
#endif // platform
    zst_string_append(&cmd_string, " ");

    int status = system(cmd_string.base);

    if (status != 0) {
        zst_log(LOG_ERROR, "run failed [%s]", cmd_string.base);
        zst_string_free(&cmd_string);
        return 1;
    }
    zst_log(LOG_GOOD, "run successfully [%s]", cmd_string.base);

    zst_string_free(&cmd_string);
    return 0;
}

ZST_DEF void zst_forger_init(zst_forger_t *forger, const char *target, const char *dir, bool is_recursive)
{
    zst_dyna_init(&forger->cmds, sizeof(zst_cmd_t), NULL);
    zst_dyna_init(&forger->objs, sizeof(zst_string_t), zst_string_free);
    zst_string_append(&forger->target, target);
    forger->count = 0;

    if (is_recursive) {
        forger->srcs = zst_fs_match_recursively(dir, "*.c");
    } else {
        forger->srcs = zst_fs_match(dir, "*.c");
    }
    for (size_t i = 0; i < forger->srcs.count; i++) {
        zst_string_t *src = zst_dyna_get(&forger->srcs, i);
#ifdef _WIN32
        zst_string_t obj = zst_string_replace(src->base, ".c", ".obj");
#else
        zst_string_t obj = zst_string_replace(src->base, ".c", ".o");
#endif
        zst_dyna_append(&forger->objs, &obj);
    }
}

static void zst_forger_update_self_(const char *cc, int argc,
        char **argv, const char *source, ...)
{
    unsigned long long t1 = zst_fs_get_timestamp(argv[0]);
    unsigned long long t2 = zst_fs_get_timestamp(source);

    if (t1 >= t2) return;

    /* backup */
    zst_string_t src = {0};
    zst_string_append(&src, "%s.old", source);
    zst_fs_copy(source, src.base);
    zst_string_t exe = {0};
    zst_string_append(&exe, "%s.old", argv[0]);
    zst_fs_copy(argv[0], exe.base);

    zst_cmd_t cmd = {0};
    zst_cmd_init(&cmd);

    /* recompile */
    zst_cmd_append_arg(&cmd, cc);
    zst_cmd_append_arg(&cmd, "-o", argv[0], source);

    /* add extra args */
    va_list ap;
    va_start(ap, source);
    char *arg;
    while ((arg = va_arg(ap, char *)) != NULL) {
        zst_cmd_append_arg(&cmd, arg);
    }
    va_end(ap);

    /* execute */
    zst_cmd_append_arg(&cmd, "&&");
    for (int i = 0; i < argc; i++) {
        zst_cmd_append_arg(&cmd, argv[i]);
    }

    zst_log(LOG_INFO, "<< Update Self >>");

    if (zst_cmd_run(&cmd) != 0) {
        /* restore and report error */
        zst_fs_move(src.base, source);
        zst_fs_move(exe.base, argv[0]);
        zst_log(LOG_FATAL, "failed to update self");
    }

    zst_fs_remove(src.base);
    zst_fs_remove(exe.base);
    zst_string_free(&src);
    zst_string_free(&exe);
    zst_cmd_free(&cmd);
    exit(EXIT_SUCCESS);
}

static void zst_forger_append_cmd_(zst_forger_t *forger, ...)
{
    va_list ap;
    va_start(ap, forger);

    zst_cmd_t *cmd = NULL;
    while ((cmd = va_arg(ap, zst_cmd_t *)) != NULL) {
        zst_dyna_append(&forger->cmds, cmd);
        forger->count++;
    }

    va_end(ap);
}

ZST_DEF void zst_forger_print(zst_forger_t *forger)
{
    for (size_t i = 0; i < forger->cmds.count; i++) {
        zst_cmd_t *cmd = zst_dyna_get(&forger->cmds, i);
        zst_string_t cmd_string = {0};

        for (size_t i = 0; i < cmd->args.count; i++) {
            zst_string_t *arg = zst_dyna_get(&cmd->args, i);
            zst_string_append(&cmd_string, " %s", arg->base);
        }

        zst_log(LOG_INFO, cmd_string.base);

        zst_string_free(&cmd_string);
    } 
}

ZST_DEF void zst_forger_free(zst_forger_t *forger)
{
    for (size_t i = 0; i < forger->cmds.count; i++) {
        zst_cmd_t *cmd = zst_dyna_get(&forger->cmds, i);
        zst_cmd_free(cmd);
    }

    zst_dyna_free(&forger->cmds);
    zst_dyna_free(&forger->srcs);
    zst_dyna_free(&forger->objs);
    zst_string_free(&forger->target);

    forger->count = 0;
}

ZST_DEF int zst_forger_run_sync(zst_forger_t *forger)
{
    for (size_t i = 0; i < forger->count; i++) {
        zst_cmd_t *cmd = zst_dyna_get(&forger->cmds, i);
        int status = zst_cmd_run(cmd);
        if (status != 0) return 1;
    }
    return 0;
}

/* MODULE: ZST_WILDCARD */

ZST_DEF bool zst_wildcard_match(const char *str, const char *pattern)
{
    if (!str || !pattern) return false;

    int m = strlen(str);
    int n = strlen(pattern);

    bool dp[m + 1][n + 1];
    
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++)
            dp[i][j] = false;
    }
    dp[0][0] = true;

    for (int j = 1; j <= n; j++) {
        if (pattern[j - 1] == '*') {
            dp[0][j] = dp[0][j - 1];
        }
    }

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (pattern[j - 1] == '*') {
                dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
            } else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            }
        }
    }

    return dp[m][n];
}

#endif // ZST_IMPLEMENTATION

#ifdef ZST_STRIP_OUT
#define wc_match                zst_wildcard_match
#define log                     zst_log
#define dyna_init               zst_dyna_init
#define dyna_append             zst_dyna_append
#define dyna_insert             zst_dyna_insert
#define dyna_remove             zst_dyna_remove
#define dyna_get                zst_dyna_get
#define dyna_free               zst_dyna_free
#define dyna_merge              zst_dyna_merge
#define string_append           zst_string_append
#define string_concat           zst_string_concat
#define string_sub              zst_string_sub
#define string_repeat           zst_string_repeat
#define string_replace          zst_string_replace
#define string_free             zst_string_free
#define stack_init              zst_stack_init
#define stack_push              zst_stack_push
#define stack_pop               zst_stack_pop
#define stack_top               zst_stack_top
#define stack_free              zst_stack_free
#define queue_init              zst_queue_init
#define queue_free              zst_queue_free
#define queue_front             zst_queue_front
#define queue_rear              zst_queue_rear
#define queue_push              zst_queue_push
#define queue_pop               zst_queue_pop
#define queue_isempty           zst_queue_isempty
#define fs_get_timestamp        zst_fs_get_timestamp
#define fs_get_name             zst_fs_get_name
#define fs_get_attr             zst_fs_get_attr
#define fs_get_size             zst_fs_get_size
#define fs_check_perm           zst_fs_check_perm
#define fs_pwd                  zst_fs_pwd
#define fs_cd                   zst_fs_cd
#define fs_move                 zst_fs_move
#define fs_copy                 zst_fs_copy
#define fs_mkdir                zst_fs_mkdir
#define fs_touch                zst_fs_touch
#define fs_remove               zst_fs_remove
#define fs_remove_all           zst_fs_remove_all
#define fs_match                zst_fs_match
#define fs_find                 zst_fs_find
#define fs_match_recursively    zst_fs_match_recursively
#define fs_find_recursively     zst_fs_find_recursively
#define fs_typeof               zst_fs_typeof
#define fs_load_file            zst_fs_load_file
#define fs_load_dir             zst_fs_load_dir
#define fs_dump_file            zst_fs_dump_file
#define fs_free_meta_file       zst_fs_free_meta_file
#define fs_free_meta_dir        zst_fs_free_meta_dir
#define fs_open_to_read         zst_fs_open_to_read
#define fs_open_to_write        zst_fs_open_to_write
#define fs_open_to_append       zst_fs_open_to_append
#define fs_close                zst_fs_close
#define fs_read                 zst_fs_read
#define fs_write                zst_fs_write
#define fs_redirect             zst_fs_redirect
#define cmdline_init            zst_cmdline_init
#define cmdline_define_flag     zst_cmdline_define_flag
#define cmdline_parse           zst_cmdline_parse
#define cmdline_usage           zst_cmdline_usage
#define cmdline_isuse           zst_cmdline_isuse
#define cmdline_get_flag        zst_cmdline_get_flag
#define cmdline_free            zst_cmdline_free
#define cmdline_dump            zst_cmdline_dump
#define flag_init               zst_flag_init
#define flag_dump               zst_flag_dump
#define flag_free               zst_flag_free
#define cmd_append_arg          zst_cmd_append_arg
#define cmd_init                zst_cmd_init
#define cmd_dump                zst_cmd_dump
#define cmd_run                 zst_cmd_run
#define cmd_free                zst_cmd_free
#define forger_append_cmd       zst_forger_append_cmd
#define forger_init             zst_forger_init
#define forger_free             zst_forger_free
#define forger_print            zst_forger_print
#define forger_run_sync         zst_forger_run_sync
#define forger_update_self      zst_forger_update_self

typedef zst_dyna_t              dyna_t;
typedef zst_string_t            string_t;
typedef zst_stack_t             stack_t;
typedef zst_queue_node_t        queue_node_t;
typedef zst_queue_t             queue_t;
typedef zst_meta_file_t         meta_file_t;
typedef zst_meta_dir_t          meta_dir_t;
typedef zst_flag_t              flag_t;
typedef zst_flag_type_t         flag_type_t;
typedef zst_cmdline_t           cmdline_t;
typedef zst_cmd_t               cmd_t;
typedef zst_forger_t            forger_t;
typedef zst_fd_t                fd_t;

#endif // ZST_STRIP_OUT
