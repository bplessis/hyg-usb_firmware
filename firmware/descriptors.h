
#ifndef PACKETS_H
#define PACKETS_H

typedef struct _setupPacketStruct {

	unsigned char bmRequestType;
	unsigned char bRequest;
	unsigned char wValue0;
	unsigned char wValue1;
	unsigned char wIndex0;
	unsigned char wIndex1;
	unsigned int wLength;
	unsigned char extra[1];

} SetupPacketStruct;

const unsigned char DeviceDescriptor[18] = {
	0x12,                   // Size of this descriptor in bytes
	0x01,                   // DEVICE descriptor type
	0x00,                   // USB Spec Release Number in BCD format LSB
	0x02,                   // USB Spec Release Number in BCD format MSB
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	0x08,                   // Max packet size for EP0 (10)
	0xd8,                   // Vendor ID LSB
	0x04,                   // Vendor ID MSB
	0xc4,                   // Product ID: Custom HID device demo LSB
	0xf2,                   // Product ID: Custom HID device demo MSB
	0x01,                   // Device release number in BCD format LSB
	0x01,                   // Device release number in BCD format MSB
	0x01,                   // Manufacturer string index
	0x02,                   // Product string index
	0x00,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

const struct {
	unsigned char bLength;
	unsigned char bDscType;
	unsigned int string[1];
} StringDescriptor0 = {
	sizeof ( StringDescriptor0 ), 0x03, {
		0x0409}                 // English
};

const struct {
	unsigned char bLength;
	unsigned char bDscType;
	unsigned int string[13];
} StringDescriptor1 = {
	sizeof ( StringDescriptor1 ), 0x03, {
		'M', 'a', 'c', 'a', 'r', 'e', 'u', 'x', '-', 'L', 'a', 'b', 's'}
};

const struct {
	unsigned char bLength;
	unsigned char bDscType;
	unsigned int string[14];
} StringDescriptor2 = {
	sizeof ( StringDescriptor2 ), 0x03, {
		'h', 'y', 'g', '-', 'u', 's', 'b', ' ', 'S', 'i', '7', '0', '2',
		'1'}
};

unsigned char *USBStringDescriptorsPtr[] = {
	( unsigned char * )&StringDescriptor0,  // Language Code
	( unsigned char * )&StringDescriptor1,  // Manufacturer
	( unsigned char * )&StringDescriptor2  // Product
};

const unsigned char ConfigurationDescriptor[32] = {

	// Configuration Descriptor

	0x09,                   // Size of this descriptor in bytes
	0x02,                   // CONFIGURATION descriptor type (0x02)
	0x20,                   // Total number of bytes in this descriptor and all the following descriptors LSB
	0x00,                   // Total number of bytes in this descriptor and all the following descriptors MSB
	0x01,                   // Number of interfaces supported by this configuration
	0x01,                   // Value used by Set Configuration to select this configuration
	0x00,                   // Index of string descriptor describing configuration - set to 0 if no string
	0b10000000,		// USB 1.0 Bus powered
	0x32,                   // MaxPower (x2mA)

	// Interface Descriptor

	0x09,                   // Size of this descriptor in bytes
	0x04,                   // INTERFACE descriptor type (0x04)
	0x00,                   // Number identifying this interface. Zero-based value.
	0x00,                   // Value used to select this alternate setting for this interface.
	0x02,                   // Number of endpoints used by this interface. Doesn't include control endpoint 0.
	0xFF,                   // Class code assigned by USB-IF
	0x00,                   // SubClass Code assigned by USB-IF
	0x00,                   // Protocol Code assigned by USB-IF
	0x00,                   // Index of string descriptor describing interface - set to 0 if no string

	// Endpoint 1 IN

	0x07,                   // Size of this descriptor in bytes
	0x05,                   // ENDPOINT descriptor type (0x05)
	0x81,                   // Endpoint Address
	0x03,                   // Attributes (Interrupt)
	EP1_BUFLEN,            // Max Packet Size LSB
	0x00,                   // Max Packet Size MSB
	0x01,                   // Interval (x1 millisecond)

	// Endpoint 1 OUT

	0x07,                   // Size of this descriptor in bytes
	0x05,                   // ENDPOINT descriptor type (0x05)
	0x01,                   // Endpoint Address
	0x03,                   // Attributes (Interrupt)
	EP1_BUFLEN,            // Max Packet Size LSB
	0x00,                   // Max Packet Size MSB
	0x01                    // Interval (x1 millisecond)
};

#endif

/* Local Variables:    */
/* mode: c             */
/* c-basic-offset: 8   */
/* indent-tabs-mode: t */
/* End:                */
