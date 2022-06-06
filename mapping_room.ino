#include <EEPROM.h>
#include <Keypad.h> //https://playground.arduino.cc/Code/Keypad/

const int ACCESS_CODE = 314; //access code for tally. no hacking!

int topFloorInfo[][] = {
                   {}, //room number
                   {} //tally count
                       };

int roomTallyBottom[] = {1,2,3,4}; //depends on the number of rooms
int roomTallyTop[] = {5,6,7,8};  //if its a constant does that mean only the size is fixed?
const int topAddr = 22; //define the starting address in EEPROM for the 
const int botAddr = 0;//top and bottom floor

void inputDestination();
int showStats();
int lightRoom();
String grabInput();
void storeData(); //stores the data in the EEprom
void populateTally(); //Populate the arrays with data stored in the EEprom
// i left the commented printlns inside the save/populate methods bc there is a way to turn certain
// code "on/off" for debugging that I will eventaully add for future purposes

const byte ROWS = 4; //Defines the number of rows and columns
const byte COLS = 4;


char waitForKey();

char hexaKeys[ROWS][COLS] = { //keyboard selection
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
 };

byte rowPins[ROWS] = {9,8,7,6};//define the connections from the arduino
byte colPins[COLS] = {5,4,3,2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);//sets up the serial monitor so we can monitor Serial code
  Serial.println("booting up...");
}

void loop() 
  {//////////////////////////////////////////////////////////////////
    Serial.println("starting");
    inputDestination(grabInput());
  }/////////////////////////////////////

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

void inputDestination(String input)
{
  Serial.println("destination called");
  int argument =  input.toInt();
  Serial.println(arg);
  boolean access = false;
  
  if(argument == ACCESS_CODE)
  {
    Serial.println("code accepted");
    access = true;  
  }
  if(access == true)
  {
    showStats();
  }
  else
  {
    Serial.println("code no accepted");
    lightRoom();
  }  
}

int showStats()
{
  Serial.println("Enter a room");
  int roomSelection =  grabInput().toInt();
  Serial.println("showing stats");
  Serial.println(roomSelection);
  if(roomExistance(roomSelection) == true)
  {
    Serial.println("room exist");
    printTally(roomSelection);
    Serial.println("printed");
  }
  else
  {
   Serial.println("room does not exist"); 
  }
  
  return roomSelection;
}

boolean roomExistance(int selection)
{
  return true;
}

void printTally(int roomSelection)
{
  Serial.print("printing: ");
  Serial.println(roomSelection);
}

int lightRoom()
{
  return 0;  
}

//https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/
//sizeof() on a array will give of the total number of bytes it has and sizeof() on a 
//individual element of that array will give of the number of bytes for index.
//therefore total-bytes/bytes-per= total number of indexes

void storeData()//I could make this a 2d array but too lake atm
  {
    for(int i = 0, j = botAddr; i < sizeof(roomTallyBottom) / sizeof(roomTallyBottom[0]) ; i++, j++)
    {
      //Serial.print(i);
      //Serial.print(roomTallyBottom[i]);
      EEPROM.write(j, roomTallyBottom[i]);  
    }
    //Serial.println();
    for(int i = 0, j = topAddr; i < (sizeof(roomTallyTop) / sizeof(roomTallyTop[0])) ; i++, j++)
    {
      //Serial.print(i);
      //Serial.print(roomTallyTop[i]);
      EEPROM.write(j, roomTallyTop[i]); 
    }
    //Serial.println();
  }

void populateTally() //when the prom is started, we need to fill in the data
{
    for(int i = botAddr; i < (sizeof( roomTallyBottom) / sizeof(roomTallyBottom[0])); i++)
    {
      //Serial.print(i);
      roomTallyBottom[i] = EEPROM.read(i); 
      //Serial.print(roomTallyBottom[i]); 
    }
    //Serial.println();
    for(int i = topAddr, j = 0; j < (sizeof(roomTallyTop) / sizeof(roomTallyTop[0])) ; i++, j++)
    {
      //Serial.print(j);
      roomTallyTop[j] = EEPROM.read(i); 
      //Serial.print(roomTallyTop[j]); 
    }
    //Serial.println();
}
