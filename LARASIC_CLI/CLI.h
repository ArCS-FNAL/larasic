#ifndef CLI_H
#define CLI_H

#if defined(WIRING) && WIRING >= 100
  #include <Wiring.h>
#elif defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

//Define a couple constants
#define RISING 1
#define FALLING 0

//Global Variables

const byte max_arg  =  6;     //Maximum number of arguments for CLI
const byte max_size = 48;     //Maximum size of buffer
static char* buf[max_arg];    //Command & Argument Buffer

//Number of control boards & LArASIC chips
const byte num_boards  = 10;  //Number of controller boards
const byte num_channel = 48;  //Number of channels per board
const byte num_global  =  3;  //Number of global registers per board

//Pin assignments for shifting out
const byte test_pin = 9;
const byte LED_pin = 11;
const byte reset_pin = 12;
const byte chip_select_pin = 13;
const byte clock_pin = 14;
const byte data_pin = 15;
const byte readback_pin = 16;

//Name of Channel Register bits
extern const char* channel_reg[8];

//Name of Global Register bits
extern const char* global_reg[4];

//LArASIC Config Array
extern byte config_array[num_boards][num_channel + num_global];
extern byte readBack[num_boards][num_channel + num_global];

//Initialize Serial Connection
//void CLI_init(int baud_rate);

// Keep track of time since restart
void incrementTicks();
void printTicks();

//Refresh prompt after command
void CLI_refresh();

//Set up pins for shifting
void shift_init();

//Read Serial Input
char readline(char readch, char* buf[]);

//Loop over shifts
void loopsShift(int counter);

//Edit a range of channel registers in config_array
void edit_channel_reg(char* buf[]);

//Edit a range of global registers in config_array
void edit_global_reg(char* buf[]);

//Print config array
void print_array(int base);
void print_readBack(int base);

//Reset config array to all zero's
void reset_array();

//Shift the array to the LArASIC's
void shift_array();

//Save config array to EEPROM
void save_array();

//Load config array from EEPROM
void load_array();

// ASIC reset
void asic_reset();

//Set a specific bit in a given channel
void set_bit(char* buf[]);

//Clear a specific bit in a given channel
void clear_bit(char* buf[]);

//Send a test pulse to the asic
void test_signal_on(char* buf[]);

//Send a test pulse to the asic
void test_signal_off(char* buf[]);

//Say whether test is ON or OFF
void test_signal();

//Display help message
void help(const char* const help_table[]);

#endif
