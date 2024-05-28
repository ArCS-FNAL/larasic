# larasic

```
python3

import serial
from serial_comm import *
ser = serial.Serial('/dev/tty.usbmodem123451', 19200, timeout=1)

ser.write(b'help\r')
printlines(ser)

ser.write(b'print\r')
printlines(ser)

# set_channels(ser)
set_channels_oneboard(ser)

ser.write(b'print\r')
printlines(ser)

###

ser.write(b'shift\r')
printlines(ser)

ser.write(b'print\r')
printlines(ser)
```

Set test channel

```
python3

import serial
from serial_comm import *
ser = serial.Serial('/dev/tty.usbmodem123451', 19200, timeout=1)

ser.write(b'help\r')
printlines(ser)

ser.write(b'print\r')
printlines(ser)

set_channels_oneboard(ser)
set_test_channel(ser, 0)

###

ser.write(b'shift\r')
printlines(ser)

ser.write(b'print\r')
printlines(ser)
```