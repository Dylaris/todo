#define ZST_IMPLEMENTATION
#include "zst.h"

#define CC          "gcc"
#define TARGET      "todo"
#define SRC_DIR     "src/"

static zst_forger_t forger = {0};

static void compile(void)
{
    /* Compile */
    for (size_t i = 0; i < forger.srcs.count; i++) {
        zst_string_t *src = (zst_string_t *) zst_dyna_get(&forger.srcs, i);
        zst_string_t *obj = (zst_string_t *) zst_dyna_get(&forger.objs, i);
        if (zst_fs_get_timestamp(src->base) <= zst_fs_get_timestamp(obj->base)) continue;
        zst_cmd_t cmd = {0};
        zst_cmd_init(&cmd);
        zst_cmd_append_arg(&cmd, CC, "-g",
                "-I", SRC_DIR, "-I", ".", "-Wall", "-Wextra",
                "-Wno-unused-function", "-c", "-o", obj->base, src->base);
        zst_forger_append_cmd(&forger, &cmd);
    }

    /* Link */
    zst_cmd_t cmd = {0};
    zst_cmd_init(&cmd);
    zst_cmd_append_arg(&cmd, CC, "-g", 
            "-I", SRC_DIR, "-I", ".", "-Wall", "-Wextra",
            "-Wno-unused-function", "-o", forger.target.base);
    for (size_t i = 0; i < forger.objs.count; i++) {
        zst_string_t *obj = (zst_string_t *) zst_dyna_get(&forger.objs, i);
        zst_cmd_append_arg(&cmd, obj->base);
    }
    zst_forger_append_cmd(&forger, &cmd);

    zst_forger_run_sync(&forger);
}

static void clean(void)
{
#ifdef _WIN32
    zst_dyna_t files = zst_fs_match_recursively(SRC_DIR, "*.obj");
#else
    zst_dyna_t files = zst_fs_match_recursively(SRC_DIR, "*.o");
#endif
    zst_fs_remove_all(&files);
    zst_fs_remove(TARGET);
    zst_dyna_free(&files);
}

static void define_flags(zst_cmdline_t *cmdl)
{
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "h", "Print this information");
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "c", "Compile all source files");
    zst_cmdline_define_flag(cmdl, FLAG_NO_ARG, "cl", "Clean all generated files");
}

int main(int argc, char **argv)
{
    zst_forger_update_self(CC, argc, argv);

    zst_cmdline_t cmdl = {0};
    zst_cmdline_init(&cmdl);
    define_flags(&cmdl);

    zst_cmdline_parse(&cmdl, argc, argv);

    zst_forger_init(&forger, TARGET, SRC_DIR, true);

    bool is_help    = zst_cmdline_isuse(&cmdl, "h");
    bool is_compile = zst_cmdline_isuse(&cmdl, "c");
    bool is_clean   = zst_cmdline_isuse(&cmdl, "cl");

    if (is_help) zst_cmdline_usage(&cmdl);
    if (is_compile) compile();
    if (is_clean) clean();

    zst_forger_free(&forger);
    zst_cmdline_free(&cmdl);

    return 0;
}

