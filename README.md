# Atari 2600+ loader

This project makes it possible to load Atari 2600 and 7800 roms to the 2600+
using a USB to serial cable or module and a Python script. Modifications to the
Atari 2600+ mainboard and I/O board are required and void your waranty.

Normally the Atari 2600+ dumper analizes the inserted cartridge and when it has
detected the cart type, the dumper pulls the CAR_TEST pin high to tell the
mainboard that a cartridge is detected and then sends the romdump over the
serial link.

The 2600+ loader takes over the dumpers job but in order to do this the
connection of the CAR_TEST and TX signals between the I/O board and mainboard
must be severed. With mod v1 a switch is added to allow switching between normal
2600+ functionality and the loader mode. A future mod v2 will do this switching
is automatically.

To sever the connection of the CARD_TEST and two TX pins between the I/O board
and mainboard these pins can be cut off (the easy method) or pushed down into
the header while heating up their respective solder pads (recommended method).

## mod-v1

For this mod an USB to serial cable or USB to serial module and a mechanical
(DPDT) switch are needed.

The USB to Serial cable or module needs to have at least GND, RX, TX and RTS
signals available. For the switch you need a DPDT switch meaning a switch with
2 rows of 3 contacts and that can be put in two positions.

to do

## mod-v2

to do
