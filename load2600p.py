################################################################################
#  Atari 2600+ ROM loader v1.01                        by Mr. Blinky Dec 2023  #
################################################################################

# requires Python 3 & pyserial to be installed

import sys
import platform
import serial
import os.path
import time

print('Atari 2600+ ROM loader v1.00 by Mr. Blinky Dec 2023\n')
print('Using python version {}\n'.format(platform.python_version()))
if  (len(sys.argv) < 3):
    print('USAGE: {} comport romfile'.format(os.path.basename(sys.argv[0])))
    sys.exit()

port = sys.argv[1]
filename = sys.argv[2]

try:
    com = serial.Serial(port, 115800, timeout = None, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, rtscts = 0)
    com.setDTR(True)  # use DTR or RTS (depends on which pin is used)
    com.setRTS(True)  # wil set CAR_TEST low to signal now cart
    time.sleep(.1)
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
  if romsize == 2048:
      rom = rom + rom
      romsize += romsize
  checksize = 3 + romsize
  if not romsize in [4096, 8192, 16384, 32768, 65536]  :
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
com.setDTR(False) # use DTR or RTS (depends on which pin is used)
com.setRTS(False) # set car_TEST high to signal cart inserted
time.sleep(.1)
com.write(data)
com.close() #keeps CAR_TEST high (DTR/RTS false)
