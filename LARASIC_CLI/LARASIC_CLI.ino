#include "CLI.h"

//Program: Command Line Interface for Programming LArASIC's
//       : Main Hub Software
//Author : Andrew Cudd
//Email  : cuddandr@msu.edu
//Date   : 6/25/14
//Version: 1.00
/////////////////////////////////////////////////////////

//Include libraries
#include <EEPROM.h>
// #include "CLI.h"
#include <TimerThree.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <avr/pgmspace.h>

extern void loopShift(int counter);

//Store the help output in program memory instead of SRAM
const char help_line1[] PROGMEM = "-----LArIAT ASIC control firmware April 7, 2015----";
const char help_line2[] PROGMEM = "print    [base] : prints the config array";
const char help_line3[] PROGMEM = "readback [base] : prints the array read back from the serial chain";
const char help_line4[] PROGMEM = "edit [value] [start] [end] : stores the value in the config array between the start and end";
const char help_line5[] PROGMEM = "global [value] [start] [end] : identical to edit, except it stores the values in the global register entries";
const char help_line6[] PROGMEM = "reset : resets the config array to all zeros";
const char help_line7[] PROGMEM = "save : stores the config array in the EEPROM";
const char help_line8[] PROGMEM = "load : loads the config array from the EEPROM";
const char help_line9[] PROGMEM = "set [bit] [start] [end] : sets the bit in the config array between start and end";
const char help_line10[] PROGMEM = "clear [bit] [start] [end] : clears the bit in the config array between start and end";
const char help_line11[] PROGMEM = "shift             send the bytes out to the ASICs";
const char help_line12[] PROGMEM = "teston [period]   turn on the test pulser, period in microseconds 1...1023";
const char help_line13[] PROGMEM = "testoff           turn off the test pulser";
const char help_line14[] PROGMEM = "test              show status of pulser on/off";
const char help_line15[] PROGMEM = "asicreset         toggle the ASIC reset line";
const char help_line16[] PROGMEM = "printTicks        Print ticks since program started";
const char help_line17[] PROGMEM = "loop [counts]     Loops over the asic_reset and shift sequences counts times";

//Table data structure for the help function
const char* const help_table[] PROGMEM = {help_line1, help_line2, help_line3, help_line4, help_line5, help_line6, help_line7, help_line8,
                                       	  help_line9, help_line10, help_line11, help_line12, help_line13, help_line14, help_line15, help_line16,
                                          help_line17};

//Name of Channel Register bits
const char* channel_reg[8] = {"STS", "SNC", "SG0", "SG1", "ST0", "ST1", "SDC", "SBF"};

//Name of Global Register bits
const char* global_reg[4] = {"S16", "STB", "STB1", "SLK"};

//LArASIC Config Array
byte config_array[num_boards][num_channel + num_global];
byte readBack[num_boards][num_channel + num_global];

void setup()
{
    //CLI_init(19200);
    Timer3.initialize(128);  // microseconds.  Actual value may depend on hardware
    Serial.begin(19200);
    CLI_refresh();
    shift_init();

   // Start timer
   Alarm.timerRepeat(1,incrementTicks);
   Serial.println("Started timer!");
}

//Run forever:
void loop()
{
   int nBytes;
   incrementTicks();

  //See if there is anything to read

    memset(buf,0,max_arg);
    if ( (readline(nBytes=Serial.read(), buf)) > 0  ) 
    {
        //Edit a range of config registers
        if(strcmp(buf[0], "edit") == 0)
            edit_channel_reg(buf);

        //Edit a range of global registers
        else if(strcmp(buf[0], "global") == 0)
            edit_global_reg(buf);

        // Print Ticks since program start
        else if(strcmp(buf[0], "printTicks") == 0)
            printTicks();

       //Print Command
        else if(strcmp(buf[0], "print") == 0)
        {
            //If no parameter is given for the base, default to HEX (base 16)
            if(buf[1] == '\0')
                buf[1] = "16";
            //Turn string into a number
            int base = strtol(buf[1], NULL, 10);
            //Print the array with the specified base
            print_array(base);
        }
        else if(strcmp(buf[0], "readback") == 0)
        {
            //If no parameter is given for the base, default to HEX (base 16)
            if(buf[1] == '\0')
                buf[1] = "16";
            //Turn string into a number
            int base = strtol(buf[1], NULL, 10);
            //Print the array with the specified base
            print_readBack(base);
        }
        else if(strcmp(buf[0], "loop") == 0)
        {
            //If no parameter is given for the base, is none
            if(buf[1] == '\0')
                buf[1] = "0";
            //Turn string into a number
            int ctr = strtol(buf[1], NULL, 10);
            // Loop over ctr times the shift sequence
            loopShift(ctr);
        }

        //Reset Command
        else if(strcmp(buf[0], "reset") == 0)
            reset_array();

        //Shift the array to the LArASIC's
        else if(strcmp(buf[0], "shift") == 0)
            shift_array();

        //Save config array to interal EEPROM
        else if(strcmp(buf[0], "save") == 0)
            save_array();

        //Load config array from interal EEPROM
        else if(strcmp(buf[0], "load") == 0)
            load_array();

        //Set a single bit of a given channel
        else if(strcmp(buf[0], "set") == 0)
            set_bit(buf);

        //Clear a single bit of a given channel
        else if(strcmp(buf[0], "clear") == 0)
            clear_bit(buf);

        //Send a test pulse to the asic
        else if(strcmp(buf[0], "teston") == 0)
            test_signal_on(buf);
            
        //Send a test pulse to the asic
        else if(strcmp(buf[0], "testoff") == 0)
            test_signal_off(buf);           

        //Send a test pulse to the asic
        else if(strcmp(buf[0], "test") == 0)
            test_signal();           

       //Send reset to the ASICs
        else if(strcmp(buf[0], "asicreset") == 0)
            asic_reset();           

        //Print help message
        else if(strcmp(buf[0], "help") == 0)
            help(help_table);

        //Catch any other command
        else
        {
          Serial.printf("\r\nCommand [%s] not recognized. Type 'help' for list of commands.", buf[0]);
        }
        delay(10);
        CLI_refresh();
    }
}
