#pragma once

#include <common.h>

struct _packed dwarf_debug_line_header {
    uint32_t length;
    uint16_t version;
    uint32_t header_length;
    uint8_t min_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    uint8_t std_opcode_lengths[12];
};
typedef struct dwarf_debug_line_header dwarf_debug_line_header_t;