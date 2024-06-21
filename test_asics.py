'''
A script to test the ASICs
'''

import argparse

from serial_comm import *

parser = argparse.ArgumentParser(
                prog='ASICTest',
                description='Script to test LArASICs')

parser.add_argument('-d', '--device', default='/dev/ttyACM0', help='Teensy USB device')
parser.add_argument('-c', '--channel', default=0, help='Channel to test')
parser.add_argument('-p', '--pulser', default=False, help='Starts the pulser')


args = parser.parse_args()

print('Using device:', args.device)

ser = serial.Serial(args.device, 19200, timeout=1)

set_channels_oneboard(ser)

if args.channel == -1:
    print('Setting all channels to TEST')
    set_all_test_channels(ser)
else:
    set_test_channel(ser, args.channel)

ser.write(b'shift\r')
printlines(ser)

ser.write(b'shift\r')
printlines(ser)

print('Reading back from the serial chain:')
ser.write(b'readback\r')
printlines(ser)

if args.pulser:
    print('Starting test pulser')
    ser.write(b'teston 200\r')
    printlines(ser)

    input('Press enter to stop')

    ser.write(b'testoff\r')
    printlines(ser)