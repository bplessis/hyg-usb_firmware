#ifndef USB_H
#define USB_H

#define PHYS_ADDR(VIRTUAL_ADDR)  pic16_linear_addr(VIRTUAL_ADDR)

#define EP0_BUFLEN 8
#define EP1_BUFLEN 8

void reset_usb (  );
void usb_interrupt_handler (  );
bool usb_in_endpoint_busy(uint8_t ep);
void usb_arm_in_transfert();
#endif
