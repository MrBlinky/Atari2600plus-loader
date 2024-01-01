################################################################################
#  Atari 2600+ ROM loader v1.02               by Mr. Blinky Dec 2023- Jan 2024 #
################################################################################

# requires Python 3 & pyserial to be installed

import sys
import platform
import serial
import os.path
import time

FIRMWARE_VERSION = '1.1' # Atari2600+ firmware version

print('Atari 2600+ ROM loader v1.02 by Mr. Blinky Dec 2023\n')
print('Using python version {}\n'.format(platform.python_version()))
if  (len(sys.argv) < 3):
    print('USAGE: {} comport romfile'.format(os.path.basename(sys.argv[0])))
    sys.exit()

port = sys.argv[1]
filename = sys.argv[2]

try:
    com = serial.Serial(port, 115800, timeout = None, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, rtscts = 0)
    com.dtr = 1  # use DTR or RTS (depends on which pin is available)
    com.rts = 1  # wil set CARD_TEST low to signal no cart
    time.sleep(.5)
    com.write(b'\xEA')
    time.sleep(.5)
except serial.SerialException:
    print('Error! Can\'t open serial port ' + port)
    sys.exit()

with open(filename, 'rb') as binfile:
  rom = bytearray(binfile.read())
  binfile.close()

a7800 = rom[1:10] == b'ATARI7800'

if a7800:
  romsize = (rom[0x31] << 24) + (rom[0x32] << 16) + (rom[0x33] << 8) + rom[0x34]
  checksize = 3 + romsize + 128
else:
  romsize = len(rom)
  if romsize == 2048 and FIRMWARE_VERSION == '1.0': # FW v1.0 needs 2K roms expanded to 4K
      rom = rom + rom
      romsize += romsize
  elif romsize == 10495: # for Pitfall II: only keep 8K rom and 2K gfx data (rom must be a multiple of 1K)
     rom = rom[:10240]
     romsize = 10240
  checksize = 3 + romsize
  if (not romsize in [4096, 8192, 16384, 32768, 65536] and FIRMWARE_VERSION == '1.0') or (romsize % 1024) :
      print("Error rom size not supported")
      sys.exit()
#create serial data      
data = bytearray([0xAA, 0x55,
       0x78 if a7800 else 0x26,
       (romsize // 1024000) * 16 + ((romsize // 102400) % 10),
       ((romsize // 10240) % 10) * 16 + ((romsize // 1024) % 10)
       ]) + rom
checksum = 0xA5
for i in range(checksize):
    checksum ^= data[2+i]
    if (checksum & 1) == 0: checksum >>= 1
data += bytearray([0x55, 0xAA, checksum, 0, 0])
#write serial data
print('Loading Atari2600+ with {} ROM: {}'.format(7800 if a7800 else 2600, os.path.basename(filename)))
com.dtr = 0 # use DTR or RTS (depends on which pin is available)
com.rts = 0 # set CARD_TEST high to signal cart inserted
time.sleep(.1)
com.write(data)
com.close() #keeps CARD_TEST high (DTR/RTS false) and cart inserted
