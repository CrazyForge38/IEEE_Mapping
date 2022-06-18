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

const int SIZE_OF_TOP_FLOOR = sizeof(top_Floor_Structure[0])/sizeof(top_Floor_Structure[0][0]);
const int SIZE_OF_BOT_FLOOR = sizeof(bot_Floor_Structure[0])/sizeof(bot_Floor_Structure[0][0]);
const int ACCESS_CODE = 314; //access code for tally. no hacking!

const int topAddr = SIZE_OF_TOP_FLOOR; //define the starting address in EEPROM for the 
const int botAddr = 0;//top and bottom floor

boolean inputDestination();
String grabInput();
void storeData(); //stores the data in the EEprom
void populateTally(); //Populate the arrays with data stored in the EEprom

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
    int floor_Num = 0;
    int room_Num = 0;
    
    Serial.println("starting");
    usrInput = grabInput(); //do we want to wait for the user to hit input?
    floor_Num = usrInput.toInt()/100;
    room_Num = usrInput.toInt();
    handling = inputDestination(usrInput); //false for access for and true for room

    if (handling == 0)
    {
      Serial.println("access code");
      usrInput = grabInput(); //do we want to wait for the user to hit input?
      floor_Num = usrInput.toInt()/100;
      room_Num = usrInput.toInt();
      if (roomExistance(room_Num) == true)
      {
        printTally(room_Num, floor_Num);  
      }
    }
    else
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

    Serial.println("This is the end of the run");

  }/////////////////////////////////////

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
          Serial.println(bot_Floor_Structure[1][i]);
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
            Serial.println(top_Floor_Structure[1][i]);
            top_Floor_Structure[1][i] = top_Floor_Structure[1][i]+1;
            Serial.println(top_Floor_Structure[1][i]);
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
  //Serial.println(argument);
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
    {
      //Serial.print(i);
      //Serial.print(bot_Floor_Structure[1][i]);
      EEPROM.write(j, bot_Floor_Structure[1][i]);  
    }
    //Serial.println();
    for(int i = 0, j = topAddr; i < SIZE_OF_TOP_FLOOR ; i++, j++)
    {
      //Serial.print(i);
      //Serial.print(" ");
      //Serial.print(top_Floor_Structure[1][i]);
      //Serial.print(" ");
      EEPROM.write(j, top_Floor_Structure[1][i]); 
    }
    //Serial.println();
  }

void populateTally() //when the prom is started, we need to fill in the data
{
    for(int i = botAddr; i < SIZE_OF_BOT_FLOOR ; i++)
    {
      //Serial.print(i);
      bot_Floor_Structure[1][i] = EEPROM.read(i); 
      //Serial.print(bot_Floor_Structure[1][i]); 
    }
    //Serial.println();
    for(int i = topAddr, j = 0; j < SIZE_OF_TOP_FLOOR ; i++, j++)
    {
      //Serial.print(j);
      top_Floor_Structure[1][j] = EEPROM.read(i); 
      //Serial.print(top_Floor_Structure[1][j]);
      //Serial.print(" "); 
    }
    //Serial.println();
}

//void testInterface()
//{
//  for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
//  {
//    Serial.print(top_Floor_Structure[1][i]);  
//  } 
//  Serial.println("");
//  Serial.println("see whats stored");
//  
//  for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
//  {
//    top_Floor_Structure[1][i] = 4;
//    Serial.print(top_Floor_Structure[1][i]);  
//  }
//  Serial.println("");
//  Serial.println("we have printed the 4 that has been stored");
//  storeData();
//  Serial.println("called storedata");
//  Serial.println("");
//
//
//  for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
//  {
//    top_Floor_Structure[1][i] = 0;
//    Serial.print(top_Floor_Structure[1][i]);  
//  } 
//  Serial.println("we have printed the new array with 0");
//  Serial.println("");
//
//  populateTally();
//  Serial.println("");
//  Serial.println("we have populated the array");
//
//  for (int i = 0; i < SIZE_OF_TOP_FLOOR; i++)
//  {
//    Serial.print(top_Floor_Structure[1][i]);  
//  } 
//}
