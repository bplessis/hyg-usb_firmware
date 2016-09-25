#include <xc.h>
#include <string.h>
#include "types.h"
#include "usb.h"
#include "leds.h"
#include "descriptors.h"
#include "main.h"

#define UOWN		0x80    // USB Ownership Bit
#define DTSEN		0x08    // Data Toggle Sync Enable
#define DTS		0x40    // Data Toggle Sync
#define BSTALL		0x04    // Buffer Stall

typedef struct {
	union {
		struct {
			/* When receiving from the SIE. (USB Mode) */
			unsigned char bch : 2;
			unsigned char pid : 4; /* See enum PID */
			unsigned char reserved: 1;
			unsigned char uown : 1;
		};
		struct {
			/* When giving to the SIE (CPU Mode) */
			unsigned char : 2;
			unsigned char bstall : 1;
			unsigned char dtsen : 1;
			unsigned char : 2;
			unsigned char dts : 1;
			unsigned char /*UOWN*/ : 1;
		};
		unsigned char value;
	} BDnSTAT;
	unsigned char BDnCNT;
	unsigned int BDnADR; /* BDnADRL and BDnADRH; */
} __BufferDescriptor;

typedef struct {
	__BufferDescriptor Output;      // Host to Device
	__BufferDescriptor Input;       // Device to Host
} Interface;

// USB SIE Memory Mapping start at 0x2000
volatile __at ( 0x2000 )
Interface Interfaces[2];

struct {
	// --- End Point 0
	unsigned char EP0_OUTbuffer[EP0_BUFLEN];
	unsigned char EP0_INbuffer[EP0_BUFLEN];

	// --- End Point 1
	unsigned char EP1_OUTbuffer[EP1_BUFLEN];
	unsigned char EP1_INbuffer[EP1_BUFLEN];
} ep_buffers __at ( 0x2080 );


// Local data
unsigned char *addr_to_send;
unsigned int cnt_to_send;

unsigned char addresse;


/* Convert a pointer, which can be a normal banked pointer or a linear
 * pointer, to a linear pointer.
 *
 * The USB buffer descriptors need linear addresses. The XC8 compiler will
 * generate banked (not linear) addresses for the arrays in ep_buffers if
 * ep_buffers can fit within a single bank. This is good for code size, but
 * the buffer descriptors cannot take banked addresses, so they must be
 * generated from the banked addresses.
 *
 * See section 3.6.2 of the PIC16F1459 datasheet for details.
 */
static unsigned int pic16_linear_addr(void *ptr)
{
	unsigned char high, low;
	unsigned int addr = (unsigned int) ptr;

	/* Addresses over 0x2000 are already linear addresses. */
	if (addr >= 0x2000)
		return addr;

	high = (addr & 0xff00) >> 8;
	low  = addr & 0x00ff;

	return 0x2000 +
	       (low & 0x7f) - 0x20 +
	       ((high << 1) + (low & 0x80)? 1: 0) * 0x50;
}

// Predecla

void setup_endpoints (  ) {

	UCONbits.PPBRST = 1;	// Hold Ping Pong while setting up endpoints

	// Reset Buffer Descriptors and re-init
	memset(Interfaces, 0x0, sizeof(Interfaces));

	// Setup endpoint buffers addresses
	Interfaces[0].Output.BDnADR = PHYS_ADDR(ep_buffers.EP0_OUTbuffer);
	Interfaces[0].Output.BDnCNT = EP0_BUFLEN;
	Interfaces[0].Output.BDnSTAT.value = UOWN | DTSEN;

	Interfaces[0].Input.BDnADR  = PHYS_ADDR(ep_buffers.EP0_INbuffer);
	Interfaces[0].Input.BDnCNT = EP0_BUFLEN;
	Interfaces[0].Input.BDnSTAT.value = 0x00;


	Interfaces[1].Output.BDnADR = PHYS_ADDR(ep_buffers.EP1_OUTbuffer);
	Interfaces[1].Output.BDnCNT = EP1_BUFLEN;
	Interfaces[1].Output.BDnSTAT.value = UOWN | DTSEN;

	Interfaces[1].Input.BDnADR  = PHYS_ADDR(ep_buffers.EP1_INbuffer);
	Interfaces[1].Input.BDnCNT = EP1_BUFLEN;
	Interfaces[1].Input.BDnSTAT.value = 0x00;

	UCONbits.PPBRST = 0;	// Reactivate Ping Pong
}

void reset_usb (  ) {

#if defined(_USB_LOWSPEED)
	UCFG = 0x10;            // Enable Pullups : Low speed device ; no ping pong
#else
	UCFG = 0x14;            // Enable Pullups : FS device ; no ping pong
#endif

	UADDR = 0x00;           // Reset usb address

	UIE = 0;		// Reset USB Interrupts
	UIE = 0b00101011;	// Enable interrupts
	//UIEbits.URSTIE = 1;	// USB Reset Interrupt
	//UIEbits.UERRIE = 1;	// USB Reset Interrupt
	//UIEbits.TRNIE  = 1;	// USB Transaction Complete
	//UIEbits.IDLEIE = 0;	// USB IDLE Detect Interrupt
	//UIEbits.STALLIE= 1;	// USB STALL Handshake Interrupt
	//UIEbits.SOFIE  = 0;	// USB Start Of Frame

#if defined(_USB_ERROR_INTERRUPT)
	UEIE = 0x9f;		// ErrorInterrupt: BTSEE | BTOEE | DFN8EE | CRC16EE | CRC5EE | PIDEE
#else
	UEIE = 0x00;		// USB Error Interrupt: disabled
#endif

	/* If the PLL is being used, wait until the
	   PLLRDY bit is set in the OSCSTAT register
	   before attempting to set the USBEN bit. */

/*	if ( UCONbits.USBEN == 0 ) {
		UCON = 0;

		while ( UCONbits.SE0 ) ;
	}
*/

	if ( OSCCONbits.SPLLEN ) {
		while (!OSCSTATbits.PLLRDY)
			NOP ( );
	}

	UCONbits.USBEN = 1;	// Enable USB module

	addresse = 0x00;
	cnt_to_send = 0;

	// Empty FIFO
	UIRbits.TRNIF = 0;
	UIRbits.TRNIF = 0;
	UIRbits.TRNIF = 0;
	UIRbits.TRNIF = 0;

	// Clear flags
	UEIR = 0x00;		// Clear all usb error interrupts
	UIR = 0x00;		// Clear all usb status interrupts

	/* These are the UEP/U1EP endpoint management registers. */

	/* Clear them all out. This is important because a bootloader
	   could have set them to non-zero */
	UEP0 = 0;UEP1 = 0; UEP2 = 0;UEP3 = 0;UEP4 = 0;UEP5 = 0;UEP6 = 0;UEP7 = 0;

	// Setup Endpoints:
	UEP0 = 0b00010110;
	//UEP0bits.EPHSHK  = 1;	// Enable Handshake
	//UEP0bits.EPOUTEN = 1;	// Outbound
	//UEP0bits.EPINEN  = 1;	// Inbound

	UEP1 = 0b00011110;
	//UEP1bits.EPHSHK   = 1; // Enable Handshake
	//UEP1bits.EPCONDIS = 1; // Enable control operations
	//UEP1bits.EPOUTEN  = 1; // Outound
	//UEP1bits.EPINEN   = 1; // Inbound

	PIE2bits.USBIE = 1;      // USB interrupt enabled

	// WaitForPacket

	setup_endpoints ();

	// Enable Packet transfers
	UCONbits.PKTDIS = 0;
}

void fill_in_buffer (  ) {

	unsigned int i;
	unsigned int byte_count;

	byte_count = (cnt_to_send < EP0_BUFLEN) ? cnt_to_send : EP0_BUFLEN;

	Interfaces[0].Input.BDnSTAT.bch = 0;
	Interfaces[0].Input.BDnCNT = byte_count;

	for ( i = 0; i < byte_count; i++ )
		ep_buffers.EP0_INbuffer[i] = addr_to_send[i];

	cnt_to_send -= byte_count;
	addr_to_send += byte_count;

}

void process_setup_packets (  ) {
	uint8_t dts;

	if ( USTATbits.DIR == 0 ) {     // This is an OUT or a SETUP transaction

		unsigned char PID = Interfaces[0].Output.BDnSTAT.pid;

		if ( PID == 0x0D ) {    // This is a SETUP packet

			// Recupere l'acces a la mémoire partagée
			// Clear UOWN & STALL
			Interfaces[0].Input.BDnSTAT.value = 0;
			Interfaces[0].Output.BDnSTAT.value = 0;

			Interfaces[0].Output.BDnCNT = EP0_BUFLEN;
			Interfaces[0].Input.BDnCNT = EP0_BUFLEN;

			SetupPacketStruct *SetupPacket;
			SetupPacket = ( SetupPacketStruct * ) ep_buffers.EP0_OUTbuffer;

			if ( SetupPacket->bmRequestType == 0x80 ) {     // Data will go from Device to host

				if ( SetupPacket->bRequest == 0x06 ) {  // GET_DESCRIPTOR

					switch ( SetupPacket->wValue1 ) {

						case 0x01:     // Device Descriptor

							addr_to_send =
								( unsigned char * )
								DeviceDescriptor;
							cnt_to_send =
								SetupPacket->wLength;

							fill_in_buffer (  );
							break;

						case 0x02:     // Configuration Descriptor

							addr_to_send =
								( unsigned char * )
								ConfigurationDescriptor;
							cnt_to_send =
								SetupPacket->wLength;

							fill_in_buffer (  );
							break;

						case 0x03:     // String descriptor
							addr_to_send =
								USBStringDescriptorsPtr
								[SetupPacket->wValue0];
							cnt_to_send = *addr_to_send;

							fill_in_buffer (  );
							break;

						default:       // Inconnu: stall ep0
							Interfaces[0].Output.BDnSTAT.value =
								UOWN | BSTALL;
							Interfaces[0].Input.BDnSTAT.value =
								UOWN | BSTALL;
							UCONbits.PKTDIS = 0;
							return;

					}
				}

				if ( SetupPacket->bRequest == 0x00 ) {  // GET_STATUS

					ep_buffers.EP0_INbuffer[0] = 0x00;
					ep_buffers.EP0_INbuffer[1] = 0x00;

					Interfaces[0].Input.BDnCNT = 2;
				}
			}

			if ( SetupPacket->bmRequestType == 0x00 ) {     // data will go from Host to Device

				switch ( SetupPacket->bRequest ) {

					case 0x05:     // SET ADDRESS
						addresse = SetupPacket->wValue0;

						break;

					case 0x09:     // SET CONFIGURATION
						setup_endpoints (  );

						break;

					default:
						break;

				}

			}

			// Rend l'acces mémoire au SIE
			Interfaces[0].Output.BDnSTAT.value = UOWN | DTSEN;
			Interfaces[0].Input.BDnSTAT.value  = UOWN | DTS | DTSEN;

			/* SETUP packet sets PKTDIS which disables
			 * future SETUP packet reception. Turn it off
			 * afer we've processed the current SETUP
			 * packet to avoid a race condition. */
			UCONbits.PKTDIS = 0;
		}

	} else {                // This is an In packet

		if ( ( addresse != 0x00 ) && ( UADDR != addresse ) )
			UADDR = addresse;

		fill_in_buffer (  );
		dts = Interfaces[0].Input.BDnSTAT.dts;
		Interfaces[0].Input.BDnSTAT.value = 0;	// Reset BDnSTAT before set

		/* NB: The firmware should not set the UOWN bit
		   in the same instruction cycle as any other
		   modifications to the BDnSTAT soft
		   register. The UOWN bit should only be set
		   in a separate instruction cycle, only after
		   all other bits in BDnSTAT (and address/
		   count registers) have been fully updated. */
		if ( dts )
			Interfaces[0].Input.BDnSTAT.value = UOWN | DTSEN;
		else
			Interfaces[0].Input.BDnSTAT.value = UOWN | DTS | DTSEN;

	}
}

void process_command_packets (  ) {
	uint8_t dts;

	if ( USTATbits.DIR == 0 ) {     // This is an OUT transaction

		if ( ep_buffers.EP1_OUTbuffer[0] == 65 )
			__dev_state.green_led = LED_ON;

		if ( ep_buffers.EP1_OUTbuffer[0] == 66 )
			__dev_state.green_led = LED_OFF;

		if ( ep_buffers.EP1_OUTbuffer[0] == 67 ) {
			set_all_leds ( 0 );
			__dev_state.green_led = LED_AUTO;
		}

		if ( ep_buffers.EP1_OUTbuffer[1] == 65 )
			__dev_state.yellow_led = LED_ON;

		if ( ep_buffers.EP1_OUTbuffer[1] == 66 )
			__dev_state.yellow_led = LED_OFF;

		if ( ep_buffers.EP1_OUTbuffer[1] == 67 ) {
			set_all_leds ( 0 );
			__dev_state.yellow_led = LED_AUTO;
		}

		if ( ep_buffers.EP1_OUTbuffer[2] == 65 )
			__dev_state.red_led = LED_ON;

		if ( ep_buffers.EP1_OUTbuffer[2] == 66 )
			__dev_state.red_led = LED_OFF;

		if ( ep_buffers.EP1_OUTbuffer[2] == 67 ) {
			set_all_leds ( 0 );
			__dev_state.red_led = LED_AUTO;
		}

		Interfaces[1].Output.BDnCNT = EP1_BUFLEN;

		dts = Interfaces[1].Output.BDnSTAT.dts;
		Interfaces[1].Output.BDnSTAT.value = 0;

		if ( dts )
			Interfaces[1].Output.BDnSTAT.value = UOWN | DTSEN;
		else
			Interfaces[1].Output.BDnSTAT.value = UOWN | DTS | DTSEN;

	} else {
		// Completed IN transaction: rearm for a next request.
		// Don't change data.
		dts = Interfaces[1].Input.BDnSTAT.dts;
		Interfaces[1].Input.BDnSTAT.value = 0;

		if ( dts )
			Interfaces[1].Input.BDnSTAT.value = UOWN | DTSEN;
		else
			Interfaces[1].Input.BDnSTAT.value = UOWN | DTS | DTSEN;

	}
}

void usb_interrupt_handler (  ) {

	if ( !PIR2bits.USBIF )
		return;

#if defined( _USB_ERROR_INTERRUPT )
	if ( UIRbits.UERRIF ) { // Detection d'erreur
		if (UEIRbits.BTSEF) { // Bit Stuff Error Flag
		}
		if (UEIRbits.BTOEF) { // Bus Turnaround Time-out Error Flag
		}
		if (UEIRbits.DFN8EF) { // Data Field Size Error Flag
		}
		if (UEIRbits.CRC16EF) { //CRC16 Failure Flag
		}
		if (UEIRbits.CRC5EF) { // CRC5 Host Error Flag
		}
		if (UEIRbits.PIDEF) { // PID Check Failure Flag bit
		}
		UEIR = 0;
	}
#endif

	if ( UIRbits.TRNIF ) {  // Transaction complete

		switch ( USTATbits.ENDP ) {
			case 0:
				process_setup_packets (  );
				break;  // EP0: Setup
			case 1:
				process_command_packets (  );
				break;  // EP1: Command v1
			default:
				break;
		}

		// Clear TRNIF, Advance USTAT FIFO
		//UIRbits.TRNIF = 0;
	}

#if 0
	if ( UIRbits.SOFIF ) {  // Start of Frame ( not used )

		//UIRbits.SOFIF = 0;
	}
#endif

	if ( UIRbits.STALLIF ) { // Stall

		if ( UEP0bits.EPSTALL == 1 ) {

			Interfaces[0].Output.BDnCNT = EP0_BUFLEN;
			Interfaces[0].Output.BDnSTAT.value = UOWN | DTSEN;

			Interfaces[0].Input.BDnSTAT.value = 0x00;

			UEP0bits.EPSTALL = 0;
		}

		if ( UEP1bits.EPSTALL == 1 ) {

			set_green_led ( 0 );
			set_yellow_led ( 0 );
			set_red_led ( 1 );

			UEP1bits.EPSTALL = 0;
		}

		//UIRbits.STALLIF = 0;
	}

	if ( UIRbits.URSTIF ) { // Reset
		reset_usb (  );
		//UIRbits.URSTIF = 0;
	}

	// Clear all bits;
	UIR = 0x00;
	PIR2bits.USBIF = 0;
}

unsigned char* usb_get_buffer () {
	return &ep_buffers.EP1_INbuffer;
}

void usb_send_in_buffer (uint8_t ep, uint8_t len) {
	Interfaces[1].Input.BDnCNT = len;

	uint8_t dts = Interfaces[1].Input.BDnSTAT.dts;
	Interfaces[1].Input.BDnSTAT.value = 0;
	if (dts)
		Interfaces[1].Input.BDnSTAT.value = UOWN | DTSEN;
	else
		Interfaces[1].Input.BDnSTAT.value = UOWN | DTS | DTSEN;
}

bool usb_in_endpoint_busy(uint8_t endpoint)
{
	return Interfaces[endpoint].Input.BDnSTAT.uown;
}

/* Local Variables:    */
/* mode: c             */
/* c-basic-offset: 8   */
/* indent-tabs-mode: t */
/* End:                */
