#include <xc.h>

#include "main.h"
#include "leds.h"
#include "usb.h"
#include "i2c.h"
#include "si7021A10.h"

// CONFIG1

#pragma config FOSC = INTOSC    // * Oscillator Selection Bits
#pragma config WDTE = OFF       // Watchdog Timer Enable
#pragma config PWRTE = OFF      // Power-up Timer Enable
#pragma config MCLRE = OFF      // MCLR Pin Function Select
#pragma config CP = OFF         // Flash Program Memory Code Protection
#pragma config BOREN = OFF      // Brown-out Reset Enable
#pragma config CLKOUTEN = OFF   // Clock Out Enable
#pragma config IESO = OFF       // Internal/External Switchover Mode
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable

// CONFIG2

#pragma config LVP = OFF        // Low-Voltage Programming Enable (Low-voltage programming enabled)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config PLLEN = ENABLED  // * PLL Enable Bit
#pragma config PLLMULT = 3x     // * PLL Multipler Selection Bit
#pragma config USBLSCLK = 24MHz // * USB Low SPeed Clock Selection bit
#pragma config CPUDIV = CLKDIV3 // * CPU System Clock Selection Bit
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)

void interrupt interrupt_handler (  ) {
        usb_interrupt_handler (  );
}

__INTERNAL_DEVSTATE __dev_state;

void main ( void ) {

        unsigned char i, j, k;

        // Set up clock for 8MHz

        OSCCON = 0xF8;

        // Activate LDO to supply power to the sensor ( RC5 )

        ANSELC = 0x00;
        TRISCbits.TRISC5 = 0;
        PORTCbits.RC5 = 1;

        // initialize measures

        __dev_state.hyg.value = 0;
        __dev_state.temp.value = 0;

        __dev_state.green_led = LED_AUTO;
        __dev_state.red_led = LED_OFF;
        __dev_state.yellow_led = LED_OFF;

        __dev_state.fill = 0;

        // setup sub systems

        setup_leds (  );

        // Animation

        set_red_led ( 1 );

        for ( j = 0; j < 255; j++ )
                for ( k = 0; k < 255; k++ )
                        NOP (  );

        set_yellow_led ( 1 );

        for ( j = 0; j < 255; j++ )
                for ( k = 0; k < 255; k++ )
                        NOP (  );

        set_green_led ( 1 );

        for ( j = 0; j < 255; j++ )
                for ( k = 0; k < 255; k++ )
                        NOP (  );

        set_green_led ( 0 );

        for ( j = 0; j < 255; j++ )
                for ( k = 0; k < 255; k++ )
                        NOP (  );

        set_yellow_led ( 0 );

        for ( j = 0; j < 255; j++ )
                for ( k = 0; k < 255; k++ )
                        NOP (  );

        set_red_led ( 0 );

        // Start USB

        setup_usb (  );

        // Measurement loop

        OpenI2C ( MASTER, SLEW_OFF );

        while ( 1 ) {

                measure_rh ( &__dev_state.hyg );
                measure_temp ( &__dev_state.temp );

                for ( i = 0; i < 3; i++ )
                        for ( j = 0; j < 255; j++ )
                                for ( k = 0; k < 255; k++ )
                                        NOP (  );

                if ( __dev_state.green_led == LED_AUTO )
                        toggle_green_led (  );
                else
                        set_green_led ( __dev_state.green_led );

                if ( __dev_state.red_led == LED_AUTO )
                        toggle_red_led (  );
                else
                        set_red_led ( __dev_state.red_led );

                if ( __dev_state.yellow_led == LED_AUTO )
                        toggle_yellow_led (  );
                else
                        set_yellow_led ( __dev_state.yellow_led );

        }
}
