# larasic


Software to control the LArASICs with the LArIAT electronics.

- `test_asics.py` script to test the electronics. Sets the test channels and starts the pulser.
- `serial_comm.py` script with functions to test the electronics.
- `LARASIC_CLI` Custom teesny firmware for testing  


Script to test the electronics:

```
python test_asics.py -c 0 -p
```
- `-c` to set the channel to test
- `-p` to start the pulser


## Troubleshooting

If the server doens't find the Teensy:
```
sudo chmod a+rw /dev/ttyACM0 
```

## Other code...

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
# ser = serial.Serial('/dev/tty.usbmodem123451', 19200, timeout=1)
ser = serial.Serial('/dev/ttyACM0', 19200, timeout=2)

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