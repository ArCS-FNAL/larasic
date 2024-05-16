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

set_channels(ser)

ser.write(b'print\r')
printlines(ser)
```