#ifndef I2C_H
#define I2C_H

#define   MASTER    			0b00001000      /* I2C Master mode */
#define   SLEW_OFF  			0b10000000      /* Slew rate disabled for 100kHz mode */
#define   SLEW_ON   			0b00000000      /* Slew rate enabled for 400kHz mode  */

#define   SSPENB    			0b00100000      /* Enable serial port and configures SCK, SDO, SDI */

#define StartI2C()  SSP1CON2bits.SEN=1;while(SSP1CON2bits.SEN)
#define StopI2C()   SSP1CON2bits.PEN=1;while(SSP1CON2bits.PEN)
#define IdleI2C()   while ((SSP1CON2 & 0x1F) | (SSP1STATbits.R_nW))
#define CloseI2C()  SSP1CON1 &=0xDF

void OpenI2C ( unsigned char sync_mode, unsigned char slew );
signed char WriteI2C ( unsigned char data_out );
unsigned char ReadI2C ( void );

void AckI2C ( void );
void NotAckI2C ( void );
void RestartI2C ( void );

#endif
