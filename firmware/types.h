#include <stdint.h>
#include <stdbool.h>

#ifndef TYPES_H
#define TYPES_H

typedef union {
        struct {
                uint8_t msb;
                uint8_t lsb;
        };
        uint16_t value;
} __WORD;

#endif
