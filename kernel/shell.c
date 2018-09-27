#include "shell.h"
#include "console.h"
#include "drivers/acpi.h"
#include "drivers/keyboard.h"
#include "drivers/ports.h"
#include "drivers/serial.h"
#include "tests/tests.h"

#include <string.h>
#include <malloc.h>
#include <vector.h>
#include <math.h>

#define KEY_BUFFER_INITIAL_SIZE 0x100
static char *key_buffer;
static size_t key_buffer_size;
static size_t key_buffer_used;
static size_t key_buffer_printed;

static vc_vector *shell_history;
static size_t shell_history_offset = 0;

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
        vc_vector_clear(shell_history);
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
    init_keyboard();
    kprint("# ");
}

void shell_read() {
    if (console_serial_enabled()) {
        while (serial_received()) {
            char c = serial_read();
            if (c == '\r' || c == '\n') {
                key_buffer_return();
            } else if (c == '\b' || c == 0x7F /* DEL */) {
                key_buffer_backspace();
            } else {
                key_buffer_append(c);
            }
        }
    }
}

void shell_handle_up() {
    size_t history_count = vc_vector_count(shell_history);
    if (++shell_history_offset > history_count) {
        shell_history_offset--;
        return;
    }

    key_buffer_set(*(char **) vc_vector_at(shell_history, history_count - shell_history_offset));
}

void shell_handle_down() {
    size_t history_count = vc_vector_count(shell_history);
    if (--shell_history_offset < 0) {
        shell_history_offset = 0;
        return;
    }

    key_buffer_set(*(char **) vc_vector_at(shell_history, history_count - shell_history_offset - 1));
}

bool key_buffer_append(const char c) {
    if (key_buffer == NULL) {
        key_buffer = malloc(key_buffer_size = KEY_BUFFER_INITIAL_SIZE);
        if (key_buffer == NULL) return false;
    } else if (key_buffer_size <= key_buffer_used + 1) {
        key_buffer = realloc(key_buffer, key_buffer_size += KEY_BUFFER_INITIAL_SIZE);
        if (key_buffer == NULL) return false;
    }

    key_buffer[key_buffer_used++] = c;
    key_buffer[key_buffer_used] = '\0';
    return true;
}

void key_buffer_backspace() {
    if (key_buffer_used > 0) {
        key_buffer[--key_buffer_used] = '\0';
    }
}

void key_buffer_clear() {
    key_buffer[key_buffer_used = 0] = '\0';
    key_buffer_printed = 0;

    // Shrink key_buffer if it expanded
    if (key_buffer_size > KEY_BUFFER_INITIAL_SIZE) {
        key_buffer = realloc(key_buffer, key_buffer_size = KEY_BUFFER_INITIAL_SIZE);
    }
}

void key_buffer_set(char *input) {
    key_buffer_used = strlen(input);
    key_buffer = realloc(key_buffer, max(key_buffer_used, KEY_BUFFER_INITIAL_SIZE));
    if (key_buffer == NULL) return; // return error of some sort?
    strncpy(key_buffer, input, key_buffer_used);
}

void key_buffer_print() {
    while (key_buffer_printed < key_buffer_used) {
        kprint_char(key_buffer[key_buffer_printed++]);
    }
    while (key_buffer_printed > key_buffer_used) {
        kprint_backspace();
        key_buffer_printed--;
    }
}

void key_buffer_return() {
    shell_callback(key_buffer);
    key_buffer_clear();
}