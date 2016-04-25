#ifndef SENSOR_7021
#define SENSOR_7021

typedef union {
        struct {
                unsigned char msb;
                unsigned char lsb;
        };
        unsigned int value;
} __DWORD;

signed char measure_rh ( __DWORD * value );
signed char measure_temp ( __DWORD * value );

#endif
