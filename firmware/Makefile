CC =  /opt/microchip/xc8/bin/xc8
MDB = /opt/microchip/mplabx/mplab_ide/bin/mdb.sh

PK2CMD = /usr/local/bin/pk2cmd
PK2DEVPATH= /usr/share/pk2

SOURCES = main.c usb.c leds.c i2c.c si7021A10.c
CHIP = 16F1455
EXEC = hygusb-firm

LDFLAGS =

OBJS = $(SOURCES:.c=.p1)

%.p1: %.c
	$(CC) --pass1 --chip=$(CHIP) $(CFLAGS)  $<

$(EXEC): $(OBJS)
	$(CC) -O$@ --chip=$(CHIP) $(OBJS) $(LDFLAGS)

all: $(EXEC)

clean:
	rm -f MPLABXLog.xml*
	rm -f l.obj
	rm -f funclist
	rm -f prog.cmd
	rm -f $(EXEC).*
	rm -f startup.*
	rm -f *.d *.p1 *.pre


pickit3: $(EXEC)
	rm -f prog.cmd
	echo "Device PIC$(CHIP)" >> prog.cmd
	echo "Hwtool PICkit3 -p" >> prog.cmd
	echo "Program $(EXEC).hex" >> prog.cmd
	echo "Quit" >> prog.cmd
	$(MDB) prog.cmd

pickit2:
	$(PK2CMD) -M -PPIC${CHIP} -F$(EXEC).hex -B$(PK2DEVPATH)
