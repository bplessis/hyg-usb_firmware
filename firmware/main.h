#ifndef MAIN_H
#define MAIN_H

#include "si7021A10.h"

typedef struct {
	__WORD hyg;
	__WORD temp;

	signed char green_led;
	signed char yellow_led;
	signed char red_led;
	uint8_t fill;

} __INTERNAL_DEVSTATE;

extern __INTERNAL_DEVSTATE __dev_state;

#endif

/* Local Variables:    */
/* mode: c             */
/* c-basic-offset: 8   */
/* indent-tabs-mode: t */
/* End:                */
