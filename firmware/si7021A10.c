#include <xc.h>

#include "types.h"
#include "i2c.h"
#include "si7021A10.h"

signed char get_i2c_dword_at ( unsigned char addr, __WORD * value ) {
        signed char ret;
        ret = 0;

        IdleI2C (  );
        StartI2C (  );
        IdleI2C (  );
        ret |= WriteI2C ( 0x80 );
        IdleI2C (  );
        ret |= WriteI2C ( addr );
        IdleI2C (  );
        RestartI2C (  );
        IdleI2C (  );
        ret |= WriteI2C ( 0x81 );
        IdleI2C (  );

        if ( !ret ) {

                value->msb = ReadI2C (  );
                AckI2C (  );
                IdleI2C (  );
                value->lsb = ReadI2C (  );
                NotAckI2C (  );

        } else {

                value->msb = 0;
                value->lsb = 0;

        }

        IdleI2C (  );
        StopI2C (  );

        return ret;
}

signed char measure_rh ( __WORD * value ) {
        return get_i2c_dword_at ( 0xE5, value );
}

signed char measure_temp ( __WORD * value ) {
        return get_i2c_dword_at ( 0xE0, value );
}
