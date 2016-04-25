#include <xc.h>
#include "i2c.h"

signed char WriteI2C ( unsigned char data_out ) {

        SSP1BUF = data_out;     // write single byte to SSP1BUF
        if ( SSP1CON1bits.WCOL )        // test if write collision occurred
                return ( -1 );  // if WCOL bit is set return negative #
        else {

                while ( SSP1STATbits.BF ) ;     // wait until write cycle is complete
                IdleI2C (  );   // ensure module is idle

                if ( SSP1CON2bits.ACKSTAT )     // test for ACK condition received
                        return -2;      // Return NACK
                else
                        return 0;       // Return ACK
        }
}

void OpenI2C ( unsigned char sync_mode, unsigned char slew ) {

        SSP1STAT &= 0x3F;       // power on state
        SSP1CON1 = 0x00;        // power on state
        SSP1CON2 = 0x00;        // power on state

        SSP1CON1 |= sync_mode;  // select serial mode
        SSP1STAT |= slew;       // slew rate on/off

        SSPADD = 0x13;

        SSP1CON1 |= SSPENB;     // enable synchronous serial port
}

unsigned char ReadI2C ( void ) {
        SSP1CON2bits.RCEN = 1;  // enable master for 1 byte reception
        while ( !SSP1STATbits.BF ) ;    // wait until byte received

        return ( SSP1BUF );
}

void AckI2C ( void ) {
        SSP1CON2bits.ACKDT = 0; // set acknowledge bit state for ACK
        SSP1CON2bits.ACKEN = 1; // initiate bus acknowledge sequence
}

void NotAckI2C ( void ) {
        SSP1CON2bits.ACKDT = 1; // set acknowledge bit for not ACK
        SSP1CON2bits.ACKEN = 1; // initiate bus acknowledge sequence
}

void RestartI2C ( void ) {
        SSP1CON2bits.RSEN = 1;  // initiate bus restart condition
}
