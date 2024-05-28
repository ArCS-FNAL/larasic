#include <EEPROM.h>
#include "CLI.h"
#include <TimerThree.h>

//void CLI_init(int baud_rate)
//{
//
//    Serial.begin(baud_rate);
//
//}

static int testIsOn = 0;
static int testPeriod = 128;
static long long ticksSinceStart = 0;
//static int nWires=384;
static int nWires=48;
  
void incrementTicks()
{
  //  Serial.println("Called incrementTicks");
  ticksSinceStart++;
}

void printTicks()
{
  incrementTicks();
  Serial.printf("\rTicks since start: %20ld\r", ticksSinceStart);
}

void asic_reset()
{
      digitalWrite(reset_pin, HIGH);
      delay(1000);			
      digitalWrite(reset_pin, LOW);
}

void CLI_refresh()
{
  //Print blank line then prompt
  //  Serial.println();
  Serial.println("");
  Serial.println("OK");
  Serial.print("\x4");
}

void shift_init()
{
  //Set up pins for shifting out data
  pinMode(test_pin, OUTPUT);
  pinMode(reset_pin, OUTPUT);
  pinMode(chip_select_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);
  pinMode(readback_pin, INPUT_PULLUP);
}

char readline(char readch, char* buffer[])
{
    //Variable Declarations
    static char pos = 0;        //Position in the buffer array
    static char msg[max_size];  //Array for raw serial input
    byte arg = 0;               //Number of arguments given
    byte i = 0;                 //Index variable
    byte rpos;                  //Position of carriage return

    //If there is a character...
    if (readch > 0)
    {
        //... do things to it.
        switch(readch)
        {
            //Process backspace characters
            case '\b':
                //Adjust position in serial array
                pos--;
                //Display the deletion of the last character
                Serial.print('\b');
                Serial.print(' ');
                Serial.print('\b');
                break;

            //Process input on carriage return (e.g. enter key)
            case '\r':
                rpos = pos;  //Set CR position
                pos = 0;     //Reset position index

                //Separate command and arguments from each other
                buffer[i] = strtok(msg, " "); //Using the space character as the separator
                while(buffer[i] != NULL)
                {

                    buffer[++i] = strtok(NULL, " ");
                }

                //Save number of arguments given
                arg = i;
                return rpos;

            //Process any other character
            default:
                //If there is still space, put the character in the array
                if(pos < max_size - 1)
                {
                    msg[pos++] = readch;  //Store character and advance position
                    msg[pos] = '\0';      //Keep array properly 'Null-terminated'
                }
                Serial.print(readch);     //Print character to screen
                break;
        }
    }
    //No end of line has been found, so return -1.
    return -1;
}

void edit_channel_reg(char* buf[])
{
    //Local Variable Declarations
    int start_index, end_index, raw_channel;
    byte card, adj_channel;

    //Get the start_index and end_index numbers
    start_index = strtol(buf[2], NULL, 10);
    //If an end_index number was entered
    if(buf[3] != NULL)
        //Store the number
        end_index = strtol(buf[3], NULL, 10);
    else
        //If no end_index was given, then set the end_index equal to the start_index.
        //This then only edits a single byte.
        end_index = start_index;

    //Check the range of the channels given
    if((start_index > -1) && (end_index < nWires))
    {
        //If range was valid, edit all of the given channels
        for(raw_channel = start_index; raw_channel <= end_index; raw_channel++)
        {
            //Map the channel number to an entry in the array
            card = ceil(raw_channel / num_channel);
            adj_channel = raw_channel % num_channel;
            //Set the corresponding entry to the value given in hex
            config_array[card][adj_channel] = static_cast<byte>(strtol(buf[1], NULL, 16));
        }
    }
    //Error message
    else
    {
        Serial.println();
        Serial.print("Invalid Channel Range");
    }
}

void edit_global_reg(char* buf[])
{
    //Local Variable Declarations
    int start_index, end_index, raw_global;
    byte card, adj_global;

    //Get the start_index and end_index numbers
    start_index = strtol(buf[2], NULL, 10);
    //If an end_index number was entered
    if(buf[3] != NULL)
        //Store the number
        end_index = strtol(buf[3], NULL, 10);
    else
        //If no end_index was given, then set the end_index equal to the start_index.
        //This then only edits a single byte.
        end_index = start_index;

    //Check the range of the channels given
    if((start_index > -1) && (end_index < 30))
    {
        //If range was valid, edit all of the given channels
        for(raw_global = start_index; raw_global <= end_index; raw_global++)
        {
            //Map the channel number to an entry in the array
            card = ceil(raw_global / num_global);
            adj_global = raw_global % num_global + 48;  //The +48 is to place the entry at the end of the row
            //Set the corresponding entry to the value given in hex
            config_array[card][adj_global] = static_cast<byte>(strtol(buf[1], NULL, 16));
        }
    }
    //Error message
    else
    {
        Serial.println();
        Serial.print("Invalid Global Range");
    }
}

void print_array(int base)
{

  Serial.println();
  for(byte i=0; i<num_boards; i++)
  {
    for(byte j=0; j<num_channel + num_global; j++)
    {
      Serial.print(config_array[i][j], base);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void print_readBack(int base)
{

  Serial.println();
  for(byte i=0; i<num_boards; i++)
  {
    for(byte j=0; j<num_channel + num_global; j++)
    {
      Serial.print(readBack[i][j], base);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void reset_array()
{
    for(byte i=0; i<num_boards; i++)
    {
        for(byte j=0; j<num_channel + num_global; j++)
            config_array[i][j] = 0;
    }
}

//Shift the array to the LArASIC's
void shift_array()
{
        //Variables to set loop indicies
        int board_start = num_boards - 1;
        int channel_start = num_channel + num_global - 1;
        byte channel_out = 0;
        int mismatch = 0;   // tally of the mismatches during readback

        digitalWrite(chip_select_pin, HIGH);
        //Outer loop through each cards (rows)
        for(int i = board_start; i >= 0; i--)
        {
            Serial.print("*** NEW BOARD:");
            Serial.println(i);

            //Inner loop through each ASIC chip (columns)
            for(int j = channel_start; j >= 0; j--)
            {
                //This block of if / else if's is to turn the original array mapping
                //into the mapping the LArASIC's are expecting. More is explained in
                //the README.

                Serial.print("*** NEW CHANNEL:");
                Serial.println(j);

                if((34 < j) && (j <= 50))
                    channel_out = j-3;
                else if((17 < j) && (j <= 33))
                    channel_out = j-2;
                else if((0 < j) && (j <= 16))
                    channel_out = j-1;
                else if(j == 34)
                    channel_out = 50;
                else if(j == 17)
                    channel_out = 49;
                else if(j == 0)
                    channel_out = 48;

                Serial.print("*** Mapped to:");
                Serial.println(channel_out);

                //digitalWrite(chip_select_pin, HIGH);
                delayMicroseconds(100);			
                byte val=config_array[i][channel_out];				
                Serial.print("config_array = ");
                Serial.println(val);
                readBack[i][channel_out] = 0;
				    for (byte k = 0; k < 8; k++)
					{
						digitalWrite(data_pin, (val & (1 << k)));
                        Serial.print("Writing on data_pin = ");
                        Serial.println((val & (1 << k)));
						//Print the value of the bit that is sent
						//if (val & (1 << k)){
						//	Serial.print(1);
						//} else {
						//    Serial.print(0);
						//}													
						delayMicroseconds(100);
						digitalWrite(clock_pin, HIGH);
						delayMicroseconds(100);
						digitalWrite(clock_pin, LOW);
						delayMicroseconds(100);
						// Print the value of the readback_pin- inverted due to circuitry
                        Serial.print("Getting from digitalRead = ");
						if (digitalRead(readback_pin)){
						  Serial.println(0);
						} else {
						  Serial.println(1);
						    readBack[i][channel_out] |= ( 1 << k );
						}
						if ((digitalRead(readback_pin)) == ((val >> k)& 1)){ 
                        Serial.println("Is mismatch");
						mismatch++; //remember- readback is inverted in circuit
						//Serial.print("*");
						}
						//Serial.println();	
					}
				// digitalWrite(chip_select_pin, LOW);
				delayMicroseconds(100);			
            }
        }
	digitalWrite(chip_select_pin, LOW);
	delayMicroseconds(100);
	digitalWrite(clock_pin,HIGH);
        Serial.println();
	print_readBack(16);
        Serial.print("Mismatches = ");
        Serial.println(mismatch);
}

void save_array()
{
    //Store each element in the EEPROM
    for(int i = 0; i < num_boards; i++)
    {
        for(int j = 0; j < num_channel + num_global; j++)
	  EEPROM.write((num_channel + num_global)*i + j, config_array[i][j]);
    }
}

void load_array()
{
    //Load each element from the EEPROM
    for(int i = 0; i < num_boards; i++)
    {
        for(int j = 0; j < num_channel + num_global; j++)
	  config_array[i][j] = EEPROM.read((num_channel + num_global)*i + j);
    }
}

void loopShift(int count)
{
  int i=0;
  for (i=0; i<count; i++)
  {
    Serial.printf("\rAttempt %d\r", i++);
    asic_reset();
    shift_array();
  }
}


void set_bit(char* buf[])
{
    //Local Variable Declarations
    int start_index, end_index, raw_channel, raw_global;
    byte card, adj_channel, adj_global;

    //Get the start_index and end_index numbers
    start_index = strtol(buf[2], NULL, 10);

    //Work-around to not have a seperate set_global bit command, and still
    //maintain the optional end argument
    if(strcmp(buf[3], "global") == 0)
    {
        buf[3] = NULL;
        buf[4] = "global";
    }

    //If an end_index number was entered
    if(buf[3] != NULL)
        //Store the number
        end_index = strtol(buf[3], NULL, 10);
    else
        //If no end_index was given, then set the end_index equal to the start_index.
        //This then only edits a single byte.
        end_index = start_index;

    //Check if global or channel bits
    if(strcmp(buf[4], "global") == 0)
    {
        if((start_index > -1) && (end_index < 30))
        {
            //If range was valid, edit all of the given channels
            for(raw_global = start_index; raw_global <= end_index; raw_global++)
            {
                //Map the channel number to an entry in the array
                card = ceil(raw_global / num_global);
                adj_global = raw_global % num_global + 48;  //The +48 is to place the entry at the end of the row

                //Check which bit to change
                if(strcmp(buf[1], global_reg[0]) == 0)
                    config_array[card][adj_global] |= B00001000;   //Set the bit by OR'ing that specific bit as a 1
                else if(strcmp(buf[1], global_reg[1]) == 0)         //with the original byte.
                    config_array[card][adj_global] |= B00000100;
                else if(strcmp(buf[1], global_reg[2]) == 0)
                    config_array[card][adj_global] |= B00000010;
                else if(strcmp(buf[1], global_reg[3]) == 0)
                    config_array[card][adj_global] |= B00000001;
                else
                {
                    //Catch any incorrect bit labels
                    Serial.println();
                    Serial.print("Bit Not Recognized");
                }
            }
        }
        //Error message
        else
            Serial.print("Invalid Channel Range");
    }

    else
    {
        //Check the range of the channels given
        if((start_index > -1) && (end_index < nWires))
        {
            //If range was valid, edit all of the given channels
            for(raw_channel = start_index; raw_channel <= end_index; raw_channel++)
            {
                //Map the channel number to an entry in the array
                card = ceil(raw_channel / num_channel);
                adj_channel = raw_channel % num_channel;

                //Check which bit to change
                if(strcmp(buf[1], channel_reg[0]) == 0)
                    config_array[card][adj_channel] |= B10000000;   //Set the bit by OR'ing that specific bit as a 1
                else if(strcmp(buf[1], channel_reg[1]) == 0)        //with the original byte.
                    config_array[card][adj_channel] |= B01000000;
                else if(strcmp(buf[1], channel_reg[2]) == 0)
                    config_array[card][adj_channel] |= B00100000;
                else if(strcmp(buf[1], channel_reg[3]) == 0)
                    config_array[card][adj_channel] |= B00010000;
                else if(strcmp(buf[1], channel_reg[4]) == 0)
                    config_array[card][adj_channel] |= B00001000;
                else if(strcmp(buf[1], channel_reg[5]) == 0)
                    config_array[card][adj_channel] |= B00000100;
                else if(strcmp(buf[1], channel_reg[6]) == 0)
                    config_array[card][adj_channel] |= B00000010;
                else if(strcmp(buf[1], channel_reg[7]) == 0)
                    config_array[card][adj_channel] |= B00000001;
                else
                {
                    //Catch any incorrect bit labels
                    Serial.println();
                    Serial.print("Bit Not Recognized");
                }
            }
        }
        //Error message
        else
            Serial.print("Invalid Channel Range");
    }
}

void clear_bit(char* buf[])
{
    //Local Variable Declarations
    int start_index, end_index, raw_channel, raw_global;
    byte card, adj_channel, adj_global;

    //Get the start_index and end_index numbers
    start_index = strtol(buf[2], NULL, 10);

    //Work-around to not have a separate clear_global bit command, and still
    //maintain the optional end argument
    if(strcmp(buf[3], "global") == 0)
    {
        buf[3] = NULL;
        buf[4] = "global";
    }

    //If an end_index number was entered
    if(buf[3] != NULL)
        //Store the number
        end_index = strtol(buf[3], NULL, 10);
    else
        //If no end_index was given, then set the end_index equal to the start_index.
        //This then only edits a single byte.
        end_index = start_index;

    //Check if global or channel bits
    if(strcmp(buf[4], "global") == 0)
    {
        if((start_index > -1) && (end_index < 30))
        {
            //If range was valid, edit all of the given channels
            for(raw_global = start_index; raw_global <= end_index; raw_global++)
            {
                //Map the channel number to an entry in the array
                card = ceil(raw_global / num_global);
                adj_global = raw_global % num_global + 48;  //The +48 is to place the entry at the end of the row

                //Check which bit to change
                if(strcmp(buf[1], global_reg[0]) == 0)
                    config_array[card][adj_global] &= B11110111;   //Clear the bit by AND'ing that specific bit as a 0
                else if(strcmp(buf[1], global_reg[1]) == 0)         //with the original byte.
                    config_array[card][adj_global] &= B11111011;
                else if(strcmp(buf[1], global_reg[2]) == 0)
                    config_array[card][adj_global] &= B11111101;
                else if(strcmp(buf[1], global_reg[3]) == 0)
                    config_array[card][adj_global] &= B11111110;
                else
                {
                    //Catch any incorrect bit labels
                    Serial.println();
                    Serial.print("Bit Not Recognized");
                }
            }
        }
        //Error message
        else
            Serial.print("Invalid Channel Range");
    }

    else
    {
        //Check the range of the channels given
        if((start_index > -1) && (end_index < nWires))
        {
            //If range was valid, edit all of the given channels
            for(raw_channel = start_index; raw_channel <= end_index; raw_channel++)
            {
                //Map the channel number to an entry in the array
                card = ceil(raw_channel / num_channel);
                adj_channel = raw_channel % num_channel;

                //Check which bit to change
                if(strcmp(buf[1], channel_reg[0]) == 0)
                    config_array[card][adj_channel] &= B01111111;   //Clear the bit by AND'ing that specific bit as a 0
                else if(strcmp(buf[1], channel_reg[1]) == 0)        //with the original byte.
                    config_array[card][adj_channel] &= B10111111;
                else if(strcmp(buf[1], channel_reg[2]) == 0)
                    config_array[card][adj_channel] &= B11011111;
                else if(strcmp(buf[1], channel_reg[3]) == 0)
                    config_array[card][adj_channel] &= B11101111;
                else if(strcmp(buf[1], channel_reg[4]) == 0)
                    config_array[card][adj_channel] &= B11110111;
                else if(strcmp(buf[1], channel_reg[5]) == 0)
                    config_array[card][adj_channel] &= B11111011;
                else if(strcmp(buf[1], channel_reg[6]) == 0)
                    config_array[card][adj_channel] &= B11111101;
                else if(strcmp(buf[1], channel_reg[7]) == 0)
                    config_array[card][adj_channel] &= B11111110;
                else
                {
                    //Catch any incorrect bit labels
                    Serial.println();
                    Serial.print("Bit Not Recognized");
                }
            }
        }
        //Error message
        else
            Serial.print("Invalid Channel Range");
    }
}

//Send a test pulse to the asic


void test_signal()
{
  if ( testIsOn )
    Serial.printf("\r\nTest pulse is ON at %d microseconds", testPeriod);
  else
    Serial.print("\r\nTest pulse is OFF");
}

void test_signal_on(char* buf[])
{
  testPeriod = strtol(buf[1],NULL,10);
  if (( testPeriod <= 0 ) || ( testPeriod > 1023 )) testPeriod = 128;
  Serial.printf("\r\nSetting testPeriod to %d microseconds", testPeriod);
  Timer3.pwm(test_pin, 255);  // Valid range is 0 .. 1023
  Timer3.setPeriod(testPeriod);
  testIsOn = 1;
}

void test_signal_off(char* buf[])
{
  Timer3.disablePwm(test_pin);
  testIsOn = 0;
}


//Display Help Message
void help(const char* const help_table[])
{
    //Buffer for string
    char help_buffer[128];

    Serial.println();
    for(int i = 0; i < 17; i++)
    {
        //Copy the corresponding line from program memory into the buffer
        strcpy_P(help_buffer, (char*)pgm_read_word(&(help_table[i])));
        Serial.println(help_buffer);
    }
}
