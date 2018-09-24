#include "console.h"
#include "drivers/acpi.h"
#include "drivers/keyboard.h"
#include "drivers/ports.h"
#include "tests/tests.h"

#include <string.h>
#include <malloc.h>
#include <vector.h>

static vc_vector *shell_history;

static void shell_callback(char *input) {
    kprint_char('\n');
    unsigned char ret = 1;
    if (strcmp(input, "exit") == 0 ||
        strcmp(input, "poweroff") == 0) {
        port_byte_out(0xf4, 0x00);
        ret = 0;
    } else if (strcmp(input, "reboot") == 0) {
        reboot();
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
        kprint("# ");
        return;
    } else if (strncmp(input, "echo ", 5) == 0) {
        kprint(input + 5);
        kprint_char('\n');
        ret = 0;
    } else if (strcmp(input, "memdbg") == 0) {
        print_chunk_debug(NULL, true);
        ret = 0;
    } else if (strncmp(input, "memdbg ", 7) == 0) {
        print_chunk_debug(input, false);
        ret = 0;
    } else if (strcmp(input, "history") == 0) {
        for (void *i = vc_vector_begin(shell_history);
             i != vc_vector_end(shell_history);
             i = vc_vector_next(shell_history, i)) {
            kprint(*(char **) i);
            kprint_char('\n');
        }
        ret = 0;
    } else if (strcmp(input, "test vector") == 0) {
        ret = (unsigned char) !vc_vector_run_tests();
    } else if (strcmp(input, "test") == 0 ||
               strncmp(input, "test ", 5) == 0) {
        kprint("Available tests:\n");
        kprint("  vector\n");
    }
    kprint_uint32(ret);
    kprint(" # ");

    if (input[0] != '\0') {
        char *value = strdup(input);
        vc_vector_push_back(shell_history, &value);
    }
}

static void shell_history_free_func(void *data) {
    free(*(char **) data);
}

void shell_init() {
    shell_history = vc_vector_create(0x100, sizeof(char *), shell_history_free_func);
    init_keyboard(shell_callback);
    kprint("# ");
}