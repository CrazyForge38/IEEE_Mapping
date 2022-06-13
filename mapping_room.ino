#include <EEPROM.h>
#include <Keypad.h> //https://playground.arduino.cc/Code/Keypad/

static int top_Floor_Structure[2][23] = {// initilazie the array only for the first run and can change throughout the program
               {102, 109, 110, 125, 126, 128, 130, 132, 133, 136, 137, 139, 140, 145, 150, 154, 155, 156, 157, 158, 159, 160, 161}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //tally count need to move this so it doesnt reset maybe
             }; //https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/

static int bot_Floor_Structure[2][23] = {// initilazie the array only for the first run and can change throughout the program
               {102, 109, 110, 125, 126, 128, 130, 132, 133, 136, 137, 139, 140, 145, 150, 154, 155, 156, 157, 158, 159, 160, 161}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //tally count need to move this so it doesnt reset maybe
             }; //https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/

const int SIZE_OF_TOP_FLOOR = sizeof(top_Floor_Structure[0])/sizeof(top_Floor_Structure[0][0]);
const int SIZE_OF_BOT_FLOOR = sizeof(bot_Floor_Structure[0])/sizeof(bot_Floor_Structure[0][0]);
const int ACCESS_CODE = 314; //access code for tally. no hacking!

int roomTallyBottom[] = {1,2,3,4}; //depends on the number of rooms
int roomTallyTop[] = {5,6,7,8};  //if its a constant does that mean only the size is fixed?
const int topAddr = 22; //define the starting address in EEPROM for the 
const int botAddr = 0;//top and bottom floor

boolean inputDestination();
//int showStats();
//int lightRoom();
String grabInput();
void storeData(); //stores the data in the EEprom
void populateTally(); //Populate the arrays with data stored in the EEprom
// i left the commented printlns inside the save/populate methods bc there is a way to turn certain
// code "on/off" for debugging that I will eventaully add for future purposes

const byte ROWS = 4; //Defines the number of rows and columns
const byte COLS = 4;

char waitForKey(); //////set up////////////

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
    String usrInput = ""; //do we have to do this for a string
    boolean handling = false;
    boolean room_Does_Exist = false;
    
    Serial.println("starting");
    usrInput = grabInput(); //do we want to wait for the user to hit input?
    Serial.println(usrInput);

    handling = inputDestination(usrInput);
    Serial.print("0 is for access: ");
    Serial.println(handling); //prints 1 if access code
    if(handling == true)//true is when the access code was accepted
      {//not the access code
        room_Does_Exist = roomExistance(usrInput.toInt());
        Serial.print(room_Does_Exist);
        Serial.println(" : 1 for it does exist");
        //incrementTally();
        //lightRoom();
      }
      else//is the access code
      {
        ////usrInput = grabInput();
        //room_Does_Exist = roomExistance(usrInput);
        //showStats();
      }

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

boolean inputDestination(String input)// 1 == true and 0 == false
{
  Serial.println("destination called");
  int argument =  input.toInt();
  Serial.println(argument);
  boolean destination = true;
  
  if(argument == ACCESS_CODE)// get ride of a if by making the lightroom() default
  {
    Serial.println("code accepted");
    destination = false;  
  }

  return destination;
}

boolean roomExistance(int room_Num)
{
  boolean found = false;
  int floor_Num;

  if(room_Num/100 == 1)
  {
    for(int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
    {
      if(room_Num == top_Floor_Structure[0][i])
      {
        found = true;  
      }
    }  
  } else {
      for(int i = 0; i < SIZE_OF_BOT_FLOOR; i++)
      {
        if(room_Num == bot_Floor_Structure[0][i])
        {
          found = true;  
        }
      }  
    }
  return found;
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
