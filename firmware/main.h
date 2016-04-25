#ifndef MAIN_H
#define MAIN_H

#include "si7021A10.h"

typedef struct {
        __DWORD hyg;
        __DWORD temp;

        signed char green_led;
        signed char yellow_led;
        signed char red_led;
        signed char fill;
} __INTERNAL_DEVSTATE;

extern __INTERNAL_DEVSTATE __dev_state;

#endif
