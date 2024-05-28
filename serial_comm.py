import serial
import time

from vars import *

def pack_channel(gain=3,
                 filt=3,
                 dcOffset=0,
                 testEnable=0,
                 outputCoupling=0,
                 outputBuffer=0):

    ret = \
    ( ( ( gain & SG_M )          << SG_S  ) | 
    ( ( filt & ST_M )          << ST_S  ) | 
    ( ( dcOffset & SNC_M )       << SNC_S ) | 
    ( ( testEnable & STS_M )     << STS_S ) |
    ( ( outputCoupling & SDC_M ) << SDC_S ) |
    ( ( outputBuffer & SDF_M )   << SDF_S ) )


    return ret

def get_word(plane='induction'):

    word = pack_channel(dcOffset=0 if plane == 'induction' else 1)

    word = hex(word)

    word = word.upper()[2:]

    print('Word is', word)

    return word


def set_channels(ser):

    print('Setting induction plane')

    word = get_word(plane='induction')

    cmd = f'edit {word} 0 {480/2-1:.0f}\r'

    ser.write(cmd.encode())
    printlines(ser)


    print('Setting collection plane')

    word = get_word(plane='collection')

    cmd = f'edit {word} {480/2:.0f} {480-1:.0f}\r'

    ser.write(cmd.encode())
    printlines(ser)


def set_channels_oneboard(ser):

    print('Setting channels for a single board')

    word = get_word(plane='collection')

    cmd = f'edit {word} 0 {48-1:.0f}\r'

    ser.write(cmd.encode())
    printlines(ser)


def set_test_channel(ser, ch):

    # Get default word
    word = get_word(plane='collection')

    # str to int
    word = int(word, 16)

    # Set test bit to 1
    word |= (1 << STS_S)

    # To hex str
    word = hex(word)

    word = word.upper()[2:]

    print('Updated word is', word)

    cmd = f'edit {word} {ch} {ch}\r'

    ser.write(cmd.encode())
    printlines(ser)




def printlines(ser):

    lines = ser.readlines()

    for line in lines:
        print(str(line))


def serial_test():

    print('Opening serial...')
    ser = serial.Serial('/dev/tty.usbmodem123451', 19200, timeout=1)
    print('...serial is open.')


    print('HELP:')
    ser.write(b'help\r')
    printlines(ser)

    print()

    print('Is TEST on?')
    ser.write(b'test\r')
    printlines(ser)


if __name__ == "__main__":

    serial_test()
