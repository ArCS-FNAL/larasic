import serial
import time

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
