#define dataPin A2 // we first start by giving the name of our signals to better 
#define latchPin A1 // refence then later on. Now we use the GPIO location to know  
#define clockPin A0 // where the signals are coming out of the microcontroler 
// the link below will explain in detial how #define works
//https://www.arduino.cc/reference/en/language/structure/further-syntax/define/

//void writeMAX7219(int LATCH,int CLOCK,int DATA,byte Address,byte Command);
//void segment_Sweep();
//void INIT_MAX7219();

void setup() {
  // put your setup code here, to run once:
  // This is where we will define our GPIO as either input or output 
  // in this case, they will be output as we are sending them from the
  // arduino to the chip
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  // Our setup is down, now lets work on the main code
  Serial.begin(9600); // thisl ine starts the infinite loop called void loop()
  //https://www.arduino.cc/reference/en/language/functions/communication/serial/
  //https://www.arduino.cc/en/serial/begin
}

void loop() {
  // put your main code here, to run repeatedly:
  // add the data sheet pic and the drawing 
  static bool setup_Display = false; //"Variables declared as static will only be created and initialized the first time a function is called."
  https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/static/
  if (setup_Display == false)
  {
    INIT_MAX7219();
    setup_Display = true;
    }
  segment_Sweep();
  
}

void segment_Sweep(){
  for(byte Address = 0x01;Address<=0x04;Address++)
  {//loop from address 1 to address 4, digits 0 through 3
    for(byte Command = 0x01; Command<=0x80 && Command != 0x00;Command*=2) // after Command reaches 128, the next *=2 will cause and over flow and make command = 0x00
    {//loop through all segments
      writeMAX7219(latchPin,clockPin,dataPin,Address,Command);
      delay(200);
    }
    writeMAX7219(latchPin,clockPin,dataPin,Address,0x00); //clears the last digit
  }

  delay(200);
  set_Decode_Mode(3); //turns decode on for digit 0-3 / 1-4
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x0C);
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x0B);
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x0D);
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x0E);
  delay(1500);
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x0F); //clears all bits
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x0F); //for digit 0-3 / 1-4
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x0F);
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x0F);
  set_Decode_Mode(0); //removes decode mode and allows to light up individual bits

  while(true)
  {
    writeMAX7219(latchPin,clockPin,dataPin,0x01,0x06);
    writeMAX7219(latchPin,clockPin,dataPin,0x02,0x4F);
    writeMAX7219(latchPin,clockPin,dataPin,0x03,0x4F);
    writeMAX7219(latchPin,clockPin,dataPin,0x04,0x4F);
  }
}

void set_Decode_Mode(int select){ // Decode means code B font which can be found in the data sheet for the MAX7219 chip
  switch (select) { //No decode means that we can light up a segment based on the bits used
  case 1:
    writeMAX7219(latchPin,clockPin,dataPin,0x09,0x00);// No decode for digits 7–0
    break;
  case 2:
    writeMAX7219(latchPin,clockPin,dataPin,0x09,0x01);// Code B decode for digit 0 No decode for digits 7–1
    break;
  case 3:
    writeMAX7219(latchPin,clockPin,dataPin,0x09,0x0F);// Code B decode for digits 3–0 No decode for digits 7–4
    break;
  case 4:
    writeMAX7219(latchPin,clockPin,dataPin,0x09,0xFF);// Code B decode for digits 7–0
    break;
  default:
    writeMAX7219(latchPin,clockPin,dataPin,0x09,0x00);// No decode for digits 7–0
    break;
}
  }

void INIT_MAX7219(){
  writeMAX7219(latchPin,clockPin,dataPin,0x0C,0x00);//Shutdown mode while setting up config.
  writeMAX7219(latchPin,clockPin,dataPin,0x0A,0x07);//Midrange Brightness Mode
  writeMAX7219(latchPin,clockPin,dataPin,0x0B,0x03);//scan only digits 0-3 i.e the ones we use
  writeMAX7219(latchPin,clockPin,dataPin,0x09,0x00);//No decode for all digits
  writeMAX7219(latchPin,clockPin,dataPin,0x04,0x00);//Write BLANK to digit 3
  writeMAX7219(latchPin,clockPin,dataPin,0x03,0x00);//Write BLANK to digit 2
  writeMAX7219(latchPin,clockPin,dataPin,0x02,0x00);//Write BLANK to digit 1
  writeMAX7219(latchPin,clockPin,dataPin,0x01,0x00);//Write BLANK to digit 0
  writeMAX7219(latchPin,clockPin,dataPin,0x0C,0x01);//Normal mode
  }

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
