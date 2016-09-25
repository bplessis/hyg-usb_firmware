#ifndef USB_H
#define USB_H

#define PHYS_ADDR(VIRTUAL_ADDR)  pic16_linear_addr(VIRTUAL_ADDR)

#define EP0_BUFLEN 8
#define EP1_BUFLEN 8

void reset_usb (  );
void usb_interrupt_handler (  );
void usb_send_in_buffer (uint8_t ep, uint8_t len);
unsigned char* usb_get_buffer ();
bool usb_in_endpoint_busy(uint8_t ep);
#endif
