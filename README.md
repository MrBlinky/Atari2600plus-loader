# Atari 2600+ loader

This project makes it possible to load Atari 2600 and 7800 roms to the 2600+
using an USB to serial cable or module and a Python script. Modifications to the
Atari 2600+ mainboard and I/O board are required and void your waranty.

Normally the Atari 2600+ dumper analizes the inserted cartridge and when it has
detected the cart type, the dumper pulls the CAR_TEST pin high to tell the
mainboard that a cartridge is detected and then sends the romdump over the
serial link.

The 2600+ loader takes over the dumpers job but in order to do this the
connection of the CAR_TEST and TX signals between the I/O board and mainboard
must be severed. With mod v1 a switch is added to allow switching between normal
2600+ functionality and the loader mode. A future mod v2 will do the switching
automatically.

To sever the connection of the CARD_TEST and two TX pins between the I/O board
and mainboard, the pins can be cut off (the easy method) or pushed down into
the header while heating up their respective solder pads (recommended method).

## Running the loader tool

The loader tool is a python script. To run it, open a commandline window. When
no Python is installed on your computer (or you don't know) enter:

```
load2600p comport romfile
```

or when Python is installed with pySerial on your computer, you can run the
script directly by entering:

```
load2600p.py comport romfile
```

**comport** is the name of the serial port of the USB serial cable/m,odule. To
find out which serial port is used, go to device manager and check under ports 
which com port is added when you connect the cable to the computer.

**romfile** is the filename of the Atari 2600 or 7800 romfile. For Atari 2600,
the rom size must be 2K, 4K, 8K, 16K, 32K and 64K any other size is not
supported (2600+ firmware v1.00). For Atari 7800 roms, the romsize may vary as
long as the rom has a 128 byte header with the correct romsize.

As of Firmware v1.1 Atari 2600 rom sizes that are a multiple of 1K are supported.

## mod-v1

For this mod an USB to serial cable or USB to serial module and a mechanical
(DPDT) switch are needed.

The USB to Serial cable or module needs to have at least GND, RX, TX and RTS
signals available. For the switch you need a DPDT switch meaning a switch with
2 rows of 3 contacts and that can be put in two positions.

Wires are soldered from the severed connections of both the I/O board and
mainboard to the switch. The TX and RTS signals of the USB serial cable/module
are also soldered tothe switch.

## mod-v2

This mod uses an 'Arduino' Pro Micro (USB-C version) that not only functions as
a USB to serial adapter but also switches automatically between cartridge and 
loader mode automatically . In addition to this the power LED can also be made
to flash when a rom is loaded over USB serial.

When using a USB-C version of the Pro Micro it is important to get a version
that has a resistor on each side of the USB-C connector (see image). These are
required for proper USB-C to USB-C cables.

TO DO
