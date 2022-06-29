#include <EEPROM.h> //arduino library
#include <Keypad.h> //https://playground.arduino.cc/Code/Keypad/
#define dataPin A2 // we first start by giving the name of our signals to better 
#define latchPin A1 // refence then later on. Now we use the GPIO location to know  
#define clockPin A0 // where the signals are coming out of the microcontroler 
// the link below will explain in detial how #define works
//https://www.arduino.cc/reference/en/language/structure/further-syntax/define/

///I hate to do this but Its easier as a global rn
static int index_Num = 0;
///

static int top_Floor_Structure[2][21] = {// initilazie the array only for the first run and can change throughout the program
               {204, 203, 202, 201, 555, 240, 241, 241, 238, 237, 234, 000, 000, 205, 214, 333, 444, 666, 000, 000, 000}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //tally count 
             }; //https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/
                ////https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/

static int bot_Floor_Structure[2][38] = {// initilazie the array only for the first run and can change throughout the program
               {109, 119, 102, 100, 155, 110, 158, 158, 000, 000, 000, 000, 000, 000, 000, 156, 125, 126, 130, 128, 555, 444, 160, 159, 000, 137, 666, 140, 150, 154, 155, 132, 133, 666, 136, 139, 000, 000}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //tally count
             }; // I can get rid of the static bc of populate and store methods

const byte ROWS = 4; //Defines the number of rows and columns
const byte COLS = 4;
char waitForKey(); //sets up the waitforkey for the keypad
char hexaKeys[ROWS][COLS] = { //keyboard selection
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
 };
byte rowPins[ROWS] = {9,8,7,6};//define the connections from the arduino
byte colPins[COLS] = {5,4,3,2};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int SIZE_OF_TOP_FLOOR = sizeof(top_Floor_Structure[0])/sizeof(top_Floor_Structure[0][0]);
const int SIZE_OF_BOT_FLOOR = sizeof(bot_Floor_Structure[0])/sizeof(bot_Floor_Structure[0][0]);
const int ACCESS_CODE = 314; //access code for tally. no hacking!
const int topAddr = SIZE_OF_TOP_FLOOR; //define the starting address in EEPROM for the top floor
const int botAddr = 0;//address of the bottom floor for the EEPROM

//boolean inputDestination(); //I thought I needed to define the function if it occurs after the main function
//String grabInput();
//void storeData(); //stores the data in the EEprom
//void populateTally(); //Populate the arrays with data stored in the EEprom

void setup() {
  Serial.begin(9600);//sets up the serial monitor so we can monitor Serial code
  Serial.println("booting up...");
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  INIT_MAX7219();

  //RESET_EEPROM(); //Call this method, comment it out after, and then run the program for inital implementation
  
  populateTally();
}
//////////////////////////////////////////////////////////
void loop() 
  {//////////////////////////////////////////////////////////////////
    String usrInput = ""; //the return of grabInput() is a string
    boolean handling = false;
    boolean room_Does_Exist = false;
    int floor_Num = 0;
    int room_Num = 0;
    //writeMAX7219(latchPin,clockPin,dataPin,0x06,0x01);
    //segment_Sweep();
    
    Serial.println("starting");
    usrInput = grabInput(); //do we want to wait for the user to hit enter?
    floor_Num = usrInput.toInt()/100;
    room_Num = usrInput.toInt();
    handling = inputDestination(usrInput); //false for access code and true for the room does exist
    
    if (handling == 0)//accepted access code 0 == false
    {
      usrReadout();
    }
    else //not access code therefore it a room number
    {
      if (roomExistance(room_Num) ==  true)
      {
        Serial.println("the room does exist");
        incrementTally(room_Num, floor_Num);
        Serial.println(index_Num);
        max7219_Interface(floor_Num, index_Num);
      } 
      else 
      {
        Serial.println("room does not exist");
      } 
    }
    storeData();
    Serial.println("This is the end of this run");
  }/////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void max7219_Interface(int floor_Num, int index_Num)
{
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x00);//Write BLANK to digit 3
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x00);//Write BLANK to digit 2
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x00);//Write BLANK to digit 1
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x00);//Write BLANK to digit 0
  writeMAX7219(latchPin,clockPin,dataPin,0x05,0x00);//Write BLANK to digit 3
  writeMAX7219(latchPin,clockPin,dataPin,0x06,0x00);//Write BLANK to digit 2
  writeMAX7219(latchPin,clockPin,dataPin,0x07,0x00);//Write BLANK to digit 1
  writeMAX7219(latchPin,clockPin,dataPin,0x08,0x00);//Write BLANK to digit 0
  
  byte max7219_Top_Floor[2][21] = {
               {0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,0x08,0x80,0x08,0x08,0x08},//Address
               {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80}//command
                         };  
  byte max7219_Bot_Floor[2][38] = {
               {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x05,0x05,},//Address
               {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x20,0x40}//command
                       }; 
  Serial.println("max info:"); 
  Serial.println();

  if(floor_Num == 1)
  {
    writeMAX7219(latchPin,clockPin,dataPin,max7219_Bot_Floor[0][index_Num],max7219_Bot_Floor[1][index_Num]);
    //writeMAX7219(latchPin,clockPin,dataPin,0x01,0x08);  
  }
  else
  {
      Serial.println(index_Num);
  Serial.println(max7219_Top_Floor[0][index_Num]);
  Serial.println(max7219_Top_Floor[1][index_Num], HEX);
    writeMAX7219(latchPin,clockPin,dataPin,max7219_Top_Floor[0][index_Num],max7219_Top_Floor[1][index_Num]);
    //writeMAX7219(latchPin,clockPin,dataPin,max7219_Top_Floor[0][index_Num],max7219_Top_Floor[1][index_Num]);
  }


}

void segment_Sweep(){
  for(byte Address = 0x01;Address<=0x04 ;Address++)
  {//loop from address 1 to address 4, digits 0 through 3
    for(byte Command = 0x01; Command<=0x80 && Command != 0x00;Command*=2) // after Command reaches 128, the next *=2 will cause and over flow and make command = 0x00
    {//loop through all segments
      writeMAX7219(latchPin,clockPin,dataPin,Address,Command);
      delay(200);
    }
    writeMAX7219(latchPin,clockPin,dataPin,Address,0x00); //clears the last digit
  }

      writeMAX7219(latchPin,clockPin,dataPin,0x01,0x80);

  delay(20000);
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x0C);
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x0B);
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x0D);
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x0E);
  delay(1500);
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x0F); //clears all bits
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x0F); //for digit 0-3 / 1-4
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x0F);
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x0F);
  while(true)
  {
    writeMAX7219(latchPin,clockPin,dataPin,0x01,0x06);
    writeMAX7219(latchPin,clockPin,dataPin,0x02,0x4F);
    writeMAX7219(latchPin,clockPin,dataPin,0x03,0x4F);
    writeMAX7219(latchPin,clockPin,dataPin,0x04,0x4F);
  }
}
///////////////////////////////////////////////////////////////////////
void INIT_MAX7219(){
  writeMAX7219(latchPin,clockPin,dataPin,0x0C,0x00);//Shutdown mode while setting up config.
  writeMAX7219(latchPin,clockPin,dataPin,0x0A,0x04);//Midrange Brightness Mode
  writeMAX7219(latchPin,clockPin,dataPin,0x0B,0x03);//scan only digits 0-3 i.e the ones we use
  writeMAX7219(latchPin,clockPin,dataPin,0x09,0x00);//No decode for all digits
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x00);//Write BLANK to digit 3
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x00);//Write BLANK to digit 2
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x00);//Write BLANK to digit 1
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x00);//Write BLANK to digit 0
  writeMAX7219(latchPin,clockPin,dataPin,0x05,0x00);//Write BLANK to digit 3
  writeMAX7219(latchPin,clockPin,dataPin,0x06,0x00);//Write BLANK to digit 2
  writeMAX7219(latchPin,clockPin,dataPin,0x07,0x00);//Write BLANK to digit 1
  writeMAX7219(latchPin,clockPin,dataPin,0x08,0x00);//Write BLANK to digit 0
  writeMAX7219(latchPin,clockPin,dataPin,0x0C,0x01);//Normal mode
  }
/////////////////////////////////////////////////////////////////////////////////////
void writeMAX7219(int LATCH,int CLOCK,int DATA,byte Address,byte Command){ //could we get ruf og LATCH and Clock bc they are global? 
  //only one thing uses it at a time, even with threading.. i think
  digitalWrite(LATCH, LOW);//the chip admits data when latch is set to low
  digitalWrite(CLOCK, LOW);//the chip stores data on the rising clock edge
  //https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/
  //shiftOut(dataPin, clockPin, bitOrder, value)
  //https://www.arduino.cc/reference/en/language/functions/advanced-io/shiftout/
  shiftOut(DATA, CLOCK, MSBFIRST, Address);
  shiftOut(DATA, CLOCK, MSBFIRST, Command);
  //Data bits are labeled D0–D15 
  //(Table 1). D8–D11 contain the register address.
  //D0–D7 contain the data, and D12–D15 are “don’t care”
  //bits. The first received is D15, the most significant bit
  //(MSB). https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
  digitalWrite(LATCH, HIGH);//the chip stores data when latch rises
}
//////////////////////////////////////////////////////////////////////////////////
void RESET_EEPROM()
{
  for(int i = 0, j = botAddr; i < SIZE_OF_BOT_FLOOR ; i++, j++)
    {//i is the index incrementer and j is the starting address
      EEPROM.write(j, 0);  
    }
    for(int i = 0, j = topAddr; i < SIZE_OF_TOP_FLOOR ; i++, j++)
    {
      EEPROM.write(j, 0); 
    }
}
////////////////////////////////////////////////////////////////////////
void usrReadout()//selecting the room to read the tally count
{
  Serial.println("access code");
  String usrInput = grabInput(); //do we want to wait for the user to hit input?
  int floor_Num = usrInput.toInt()/100;
  int room_Num = usrInput.toInt();
  if (roomExistance(room_Num) == true)
    {
      printTally(room_Num, floor_Num);  
    }
}
////////////////////////////////////////////////////////////////////
void printTally(int room_Num, int floor_Num)
{
  if(floor_Num == 1)
  {
    for (int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
    {
      if (room_Num == bot_Floor_Structure[0][i])
      {
        Serial.println(bot_Floor_Structure[1][i]);  
      }  
    }  
  }
  else
  {
    for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
    {
      if (room_Num == top_Floor_Structure[0][i])
      {
        Serial.println(top_Floor_Structure[1][i]);  
      }  
    } 
  }
  Serial.println("printed tally");
}
/////////////////////////////////////////////////////////////////////
void incrementTally(int room_Num, int floor_Num)
{
  switch (floor_Num){
    case 1: 
      for (int i =0; i < SIZE_OF_BOT_FLOOR; i++)
      {
        if(room_Num == bot_Floor_Structure[0][i])
        {
          bot_Floor_Structure[1][i] = bot_Floor_Structure[1][i]+1;
          Serial.println(bot_Floor_Structure[1][i]);
        }  
      } 
    break;
    
    case 2:
      for (int i =0; i < SIZE_OF_TOP_FLOOR; i++)
        {
          if(room_Num == top_Floor_Structure[0][i])
          {
            top_Floor_Structure[1][i] = top_Floor_Structure[1][i]+1;
            Serial.println(top_Floor_Structure[1][i]);
          }  
        } 
    break;
  }// I need a defaul, we will look into this later
}
///////////////////////////////////////////////////////////////////
String grabInput() //I could add a hit '' to enter the value
{
  char char_ ;
  String usrInput = "";
  for (int i = 0; i < 3; i++)
  {
    char_ = customKeypad.waitForKey();
    Serial.println(char_);
    usrInput.concat(char_);
  }
  return usrInput;
}
////////////////////////////////////////////////////////////////////////
boolean inputDestination(String input)// 1 == true and 0 == false
{
  Serial.println("destination called");
  int argument =  input.toInt();
  boolean destination = true;
  
  if(argument == ACCESS_CODE)// get ride of a if by making the lightroom() default
  {
    Serial.println("code accepted");
    destination = false;  
  }

  return destination;
}
///////////////////////////////////////////////////////////
boolean roomExistance(int room_Num)
{
  boolean found = false;
  int floor_Num;

  if(room_Num/100 == 2)
  {
    for(int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
    {
      if(room_Num == top_Floor_Structure[0][i])
      {
        index_Num = i;
        found = true;  
      }
    }  
  } else {
      for(int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
      {
        if(room_Num == bot_Floor_Structure[0][i])
        {
          index_Num = i;
          found = true;  
        }
      }  
    }
  return found;
}
////////////////////////////////////////////////////////////////////
void storeData()//I could make this a 2d array but too late atm
  {
    for(int i = 0, j = botAddr; i < SIZE_OF_BOT_FLOOR ; i++, j++)
    {//i is the index incrementer and j is the starting address
      EEPROM.write(j, bot_Floor_Structure[1][i]);  
    }
    for(int i = 0, j = topAddr; i < SIZE_OF_TOP_FLOOR ; i++, j++)
    {//i is the index incrementer and j is the starting address
      EEPROM.write(j, top_Floor_Structure[1][i]); 
    }
  }
////////////////////////////////////////////////////////////////////////////////
void populateTally() //when the prom is started, we need to fill in the data
{
    for(int i = botAddr; i < SIZE_OF_BOT_FLOOR ; i++)
    {
      bot_Floor_Structure[1][i] = EEPROM.read(i);  
    }
    for(int i = topAddr, j = 0; j < SIZE_OF_TOP_FLOOR ; i++, j++)
    {
      top_Floor_Structure[1][j] = EEPROM.read(i); 
    }
}
