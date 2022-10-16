#include <EEPROM.h> //arduino library
#include <Keypad.h> //https://playground.arduino.cc/Code/Keypad/
#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h> //Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_Alphanumeric_Display by SparkFun
#include <SparkFun_Qwiic_Keypad_Arduino_Library.h>
HT16K33 display;
#define dataPin A2 // we first start by giving the name of our signals to better 
#define latchPin A1 // refence then later on. Now we use the GPIO location to know  
#define clockPin A0 // where the signals are coming out of the microcontroler 
// the link below will explain in detial how #define works
//https://www.arduino.cc/reference/en/language/structure/further-syntax/define/

const byte interruptPin = 2;
volatile byte state = LOW;
///I hate to do this but Its easier as a global rn if i doont want to run through a n lenght for loop..
static int index_Num = 0;

static int top_Floor_Structure[2][33] = {// initilazie the array only for the first run and can change throughout the program
  {204, 203, 202, 201, 555, 240, 241, 242,   238, 239, 237, 221, 223, 226, 231, 233, 234, 236,     205, 214, 444, 555, 666, 220, 222, 224, 225, 227, 228, 229, 230, 232, 235}, //room number
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //tally count
}; //https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/
////https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/

static int bot_Floor_Structure[2][72] = {// initilazie the array only for the first run and can change throughout the program
  {124, 123, 122, 121, 120, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 156,        119, 119, 102, 101, 103, 555, 161, 162, 158, 158,      125, 126, 130, 128, 555, 444, 160, 159,        100, 137, 666, 140, 150, 154, 155,       132, 133, 555, 136, 139, 145, 146, 147, 148, 149, 151, 152, 153}, //room number
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //tally count
}; // I can get rid of the static bc of populate and store methods
KEYPAD keypad1; //Create instance of this object
char button = 0; //Button '0' to '9' and '*' and '#'

const int SIZE_OF_TOP_FLOOR = sizeof(top_Floor_Structure[0]) / sizeof(top_Floor_Structure[0][0]);
const int SIZE_OF_BOT_FLOOR = sizeof(bot_Floor_Structure[0]) / sizeof(bot_Floor_Structure[0][0]);
const int ACCESS_CODE = 314; //access code for tally. no hacking!
const int topAddr = SIZE_OF_TOP_FLOOR; //define the starting address in EEPROM for the top floor
const int botAddr = 0;//address of the bottom floor for the EEPROM

void setup() {
  Serial.begin(9600);//sets up the serial monitor so we can monitor Serial code
  Serial.println("booting up...");
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Wire.begin(); //Join I2C bus

  if (display.begin() == false)
  {
    Serial.println("Device did not acknowledge! Freezing.");
    while (1);
  }

  INIT_MAX7219();
  display.setBrightness(7);

  //RESET_EEPROM(); //Call this method, comment it out after, and then run the program for inital implementation

  populateTally();

  Serial.println("Qwiic KeyPad Example");//////////////////////// some of this is not needed

  if (keypad1.begin() == false)   // Note, using begin() like this will use default I2C address, 0x4B.
    // You can pass begin() a different address like so: keypad1.begin(Wire, 0x4A).
  {
    Serial.println("Keypad does not appear to be connected. Please check wiring. Freezing...");
    while (1);
  }
  Serial.print("Initialized. Firmware Version: "); //this stuff isnt
  Serial.println(keypad1.getVersion());
  Serial.println("Press a button: * to do a space. # to go to next line.");//////////////////////////
}
//////////////////////////////////////////////////////////
void loop()
{ //////////////////////////////////////////////////////////////////
  keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
  button = keypad1.getButton();
  String usrInput = ""; //the return of grabInput() is a string
  boolean handling = false;
  boolean room_Does_Exist = false;
  int floor_Num = 0;
  int room_Num = 0;

  Serial.println("starting");
  attachInterrupt(digitalPinToInterrupt(interruptPin), storeData, RISING); ///// interrupt
  usrInput = grabInput(); //do we want to wait for the user to hit enter?
  floor_Num = usrInput.toInt() / 100;
  room_Num = usrInput.toInt();
  handling = inputDestination(usrInput); //false for access code and true for the room does exist

  if (handling == 0)//accepted access code 0 == false
  {
    usrReadout();
    handling = 1;
  }
  else //not access code therefore it a room number
  {
    if (roomExistance(room_Num) ==  true)
    {
      Serial.println("the room does exist");
      incrementTally(room_Num, floor_Num);
      display.print(room_Num);
      max7219_Interface(floor_Num, index_Num, room_Num);
    }
    else
    {
      String RDNE = "ROOM DOES NOT EXIST";
      //scroll_Text(RDNE);
      display.print("RDNE");
      Serial.println("room does not exist");
    }
  }
  //storeData();
  delay(2300);
  Serial.println("This is the end of this run");
}/////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void max7219_Interface(int floor_Num, int index_Num, int room_Num)
{
  for (int i = 0; i <= 8; i++)
  {
    writeMAX7219(latchPin, clockPin, dataPin, i, 0x00); //Write BLANK to all digits
  }

  byte max7219_Top_Floor[2][33] = {
    {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,         0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,           0x08, 0x08, 0x08, 0x08, 0x80, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, //Address
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,         0x01, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x10,           0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80} //command
  };
  byte max7219_Bot_Floor[2][72] = {
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,     0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,   0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05}, //Address
    {0x03, 0x03, 0x04, 0x08, 0x08, 0x10, 0x20, 0x20, 0xC0, 0xC0,   0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x80,     0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,   0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40} //command
  };

  if (floor_Num == 1)
  {
    writeMAX7219(latchPin, clockPin, dataPin, max7219_Bot_Floor[0][index_Num], max7219_Bot_Floor[1][index_Num]);
    //writeMAX7219(latchPin,clockPin,dataPin,0x01,0x08);
  }
  else if (floor_Num == 2)
  {
    writeMAX7219(latchPin, clockPin, dataPin, max7219_Top_Floor[0][index_Num], max7219_Top_Floor[1][index_Num]);
  }
  else if (room_Num == 555) //lights up all stairs
  {
    writeMAX7219(latchPin, clockPin, dataPin, 0x02, 0x10);
    writeMAX7219(latchPin, clockPin, dataPin, 0x03, 0x10);
    writeMAX7219(latchPin, clockPin, dataPin, 0x06, 0x10);
    writeMAX7219(latchPin, clockPin, dataPin, 0x08, 0x04);
  }
  else if (room_Num == 444) //lights up all restrooms
  {
    writeMAX7219(latchPin, clockPin, dataPin, 0x03, 0x20);
    writeMAX7219(latchPin, clockPin, dataPin, 0x04, 0x04);
    writeMAX7219(latchPin, clockPin, dataPin, 0x08, 0x10);
  }
  else if (room_Num == 666) //lights up all Elevators
  {
    writeMAX7219(latchPin, clockPin, dataPin, 0x03, 0x20);
    writeMAX7219(latchPin, clockPin, dataPin, 0x08, 0x08);
  }

}

///////////////////////////////////////////////////////////////////////
void INIT_MAX7219() {
  writeMAX7219(latchPin, clockPin, dataPin, 0x0C, 0x00); //Shutdown mode while setting up config.
  writeMAX7219(latchPin, clockPin, dataPin, 0x0A, 0x04); //Midrange Brightness Mode
  writeMAX7219(latchPin, clockPin, dataPin, 0x0B, 0x07); //scan only digits 0-7 i.e the ones we use
  writeMAX7219(latchPin, clockPin, dataPin, 0x09, 0x00); //No decode for all digits
  for (int i = 0; i <= 8; i++)
  {
    writeMAX7219(latchPin, clockPin, dataPin, i, 0x00); //Write BLANK to all digits
  }
  writeMAX7219(latchPin, clockPin, dataPin, 0x0C, 0x01); //Normal mode
}
/////////////////////////////////////////////////////////////////////////////////////
void writeMAX7219(int LATCH, int CLOCK, int DATA, byte Address, byte Command) { //could we get rid of LATCH and Clock bc they are global?
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
  for (int i = 0, j = botAddr; i < SIZE_OF_BOT_FLOOR ; i++, j++)
  { //i is the index incrementer and j is the starting address
    EEPROM.write(j, 0);
  }
  for (int i = 0, j = topAddr; i < SIZE_OF_TOP_FLOOR ; i++, j++)
  {
    EEPROM.write(j, 0);
  }
}
////////////////////////////////////////////////////////////////////////
void usrReadout()//selecting the room to read the tally count
{
  Serial.println("access code");
  String usrInput = grabInput(); //do we want to wait for the user to hit input?
  int floor_Num = usrInput.toInt() / 100;
  int room_Num = usrInput.toInt();
  if (roomExistance(room_Num) == true)
  {
    printTally(room_Num, floor_Num);
    display.print(room_Num);
  }
}
////////////////////////////////////////////////////////////////////
void printTally(int room_Num, int floor_Num)
{
  if (floor_Num == 1)
  {
    for (int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
    {
      if (room_Num == bot_Floor_Structure[0][i])
      {
        Serial.println(bot_Floor_Structure[1][i]);
        display.print(bot_Floor_Structure[1][i]);
        delay(2300);
      }
    }
  }
  else
  {
    for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
    {
      if (room_Num == top_Floor_Structure[0][i])
      {
        Serial.println(top_Floor_Structure[0][i]);
        display.print(top_Floor_Structure[0][i]);
        delay(2300);
      }
    }
  }
  Serial.println("printed tally");
}
/////////////////////////////////////////////////////////////////////
void incrementTally(int room_Num, int floor_Num)
{
  switch (floor_Num) {
    case 1:
      for (int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
      {
        if (room_Num == bot_Floor_Structure[0][i])
        {
          bot_Floor_Structure[1][i] = bot_Floor_Structure[1][i] + 1;
          //Serial.println(bot_Floor_Structure[1][i]);
          //display.print(bot_Floor_Structure[1][i]);
          //delay(2300);
        }
      }
      break;

    case 2:
      for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
      {
        if (room_Num == top_Floor_Structure[0][i])
        {
          top_Floor_Structure[1][i] = top_Floor_Structure[1][i] + 1;
          //Serial.println(top_Floor_Structure[1][i]);
          //display.print(bot_Floor_Structure[1][i]);
          //delay(2300);
        }
      }
      break;
  }// I need a default, we will look into this later
}
///////////////////////////////////////////////////////////////////
String grabInput()
{ /////////////////////////////////////// using the keypad
  String EARN = "ENTER A ROOM NUMBER";
  scroll_Text(EARN);
  //display.shiftLeft(grab_Input); //if this does not work, we will do a method called scroll text
  keypad1.updateFIFO();
  display.print("EARN");
  char char_ = keypad1.getButton();
  int button_Count = 0;
  String Buffer = "";

  while (button_Count < 3) {
    delay(40);
    keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
    char button = keypad1.getButton();
    if (button != 0)
    {
      if (button == '#') Serial.println();
      else if (button == '*') Serial.print(" ");
      else Serial.print(button);
      Buffer.concat(button);
      display.print(Buffer);
      button_Count++;
    }
    delay(25); //25 is good, more is better
  }
  //Serial.print("hello");
  return Buffer;
}
///////////////////////////////////////////////////////////////////////
void scroll_Text (String text)
{
  String string_Spaces = "    ";
  String text_Cat = "";
  int size_Count;

  text.concat(string_Spaces);
  string_Spaces.concat(text);
  size_Count = string_Spaces.length();

  for (int i = 0; i < (size_Count - 4); i++) {
    display.print(string_Spaces.substring(i, i + 4));
    delay(350); //idk if this is a good time
  }
}
////////////////////////////////////////////////////////////////////////
boolean inputDestination(String input)// 1 == true and 0 == false
{
  Serial.println("destination called");
  int argument =  input.toInt();
  boolean destination = true;

  if (argument == ACCESS_CODE) // get ride of a if by making the lightroom() default
  {
    Serial.println("code accepted");
    String ACA = "ACCESS CODE ACCEPTED";
    //scroll_Text(RDNE);
    display.print("ACA");
    delay(2300);
    destination = false;
  }

  return destination;
}
///////////////////////////////////////////////////////////
boolean roomExistance(int room_Num)
{
  boolean found = false;
  int floor_Num;

  if (room_Num / 100 == 2)
  {
    for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
    {
      if (room_Num == top_Floor_Structure[0][i])
      {
        index_Num = i;
        found = true;
      }
    }
  } else if (room_Num / 100 == 1)
  {
    for (int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
    {
      if (room_Num == bot_Floor_Structure[0][i])
      {
        index_Num = i;
        found = true;
      }
    }
  }
  else if (room_Num == 444 || room_Num == 555 || room_Num == 666)
  {
    found = true;
  }
  return found;
}
////////////////////////////////////////////////////////////////////
void storeData()//I could make this a 2d array but too late atm
{
  Serial.println("testing interrupt mapping");
  for (int i = 0, j = botAddr; i < SIZE_OF_BOT_FLOOR ; i++, j++)
  { //i is the index incrementer and j is the starting address
    //Serial.println(bot_Floor_Structure[1][i]);
    EEPROM.write(j, bot_Floor_Structure[1][i]);
  }
  for (int i = 0, j = topAddr; i < SIZE_OF_TOP_FLOOR ; i++, j++)
  { //i is the index incrementer and j is the starting address
    Serial.println(top_Floor_Structure[1][i]);
    EEPROM.write(j, top_Floor_Structure[1][i]);
  }
}
////////////////////////////////////////////////////////////////////////////////
void populateTally() //when the prom is started, we need to fill in the data
{
  for (int i = botAddr; i < SIZE_OF_BOT_FLOOR ; i++)
  {
    bot_Floor_Structure[1][i] = EEPROM.read(i);
  }
  for (int i = topAddr, j = 0; j < SIZE_OF_TOP_FLOOR ; i++, j++)
  {
    top_Floor_Structure[1][j] = EEPROM.read(i);
  }
}
