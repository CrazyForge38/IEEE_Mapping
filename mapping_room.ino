#include <EEPROM.h> //arduino library
#include <Keypad.h> //https://playground.arduino.cc/Code/Keypad/

static int top_Floor_Structure[2][23] = {// initilazie the array only for the first run and can change throughout the program
               {102, 109, 110, 125, 126, 128, 130, 132, 133, 136, 137, 139, 140, 145, 150, 154, 155, 156, 157, 158, 159, 160, 161}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //tally count 
             }; //https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/
                ////https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/

static int bot_Floor_Structure[2][23] = {// initilazie the array only for the first run and can change throughout the program
               {102, 109, 110, 125, 126, 128, 130, 132, 133, 136, 137, 139, 140, 145, 150, 154, 155, 156, 157, 158, 159, 160, 161}, //room number
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //tally count
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

  //RESET_EEPROM(); //Call this method, comment it out after, and then run the program for inital implementation
  
  populateTally();
}

void loop() 
  {//////////////////////////////////////////////////////////////////
    String usrInput = ""; //the return of grabInput() is a string
    boolean handling = false;
    boolean room_Does_Exist = false;
    int floor_Num = 0;
    int room_Num = 0;
    
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
        //lightRoom(room_Num);
      } 
      else 
      {
        Serial.println("room does not exist");
      } 
    }
    storeData();
    Serial.println("This is the end of this run");
  }/////////////////////////////////////

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
          }  
        } 
    break;
  }// I need a defaul, we will look into this later
}

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
