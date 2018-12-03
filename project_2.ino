int data = 3; 
int clock = 5;
int latch = 4;

int pinLED = 8;

int selectButtonPin = 9;
int changeButtonPin = 10;
int button1State = 0;// variable for reading the pushbutton status
int button2State = 0;

//Used for single LED manipulation
volatile int ledState = 0;
const int ON = HIGH;
const int OFF = LOW;
bool on = false;

int selectedLED = 0;
bool mineTurn = true;
int boardState[2][9];

void setup()
{
  Serial.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);  
  pinMode(pinLED,OUTPUT);
  pinMode(selectButtonPin,INPUT);
  pinMode(changeButtonPin,INPUT);
  for(int i = 0;i<7;i++)
  {
    changeLED(i,OFF);
  }
  digitalWrite(pinLED,LOW);
}



void loop()
{
  if(mineTurn)
  {
    button1State = digitalRead(selectButtonPin);
    button2State = digitalRead(changeButtonPin);
    if(button1State == LOW)
    {
      if(boardState[0][selectedLED] == 1||boardState[1][selectedLED] == 1)
      {        
      }
      else
      {
        if(selectedLED==8)
        {
          digitalWrite(pinLED,HIGH);
        }
        else
        {
          changeLED(selectedLED,ON);
        }
        boardState[0][selectedLED]++;
        button1State = HIGH;
        Serial.write(selectedLED);
        mineTurn = false;
      }
    }
    if(button2State == LOW)
    {
      selectedLED  = (selectedLED+1) % 9;
      button2State = HIGH;
    }
  }
  else
  {
    if(Serial.available()>0)
    {
      selectedLED = Serial.read();
      if(selectedLED==8)
        {
          digitalWrite(pinLED,HIGH);
        }
        else
        {
          changeLED(selectedLED,ON);
        }
      boardState[1][selectedLED]++;
      mineTurn = true;
    }
  }
}

bool CheckWin()
{
  if(boardState[0][0]==1)
  {
    if(boardState[0][1]==1||boardState[0][2]==1)
    {
      return true;
    }
    if(boardState[0][3]==1||boardState[0][6]==1)
    {
      return true;
    }
    if(boardState[0][4]==1||boardState[0][8]==1)
    {
      return true;
    }
  }
  else if(boardState[0][5]==1)
  {
    if(boardState[0][2]==1||boardState[0][8]==1)
    {
      return true;
    }
    if(boardState[0][3]==1||boardState[0][4]==1)
    {
      return true;
    }
  }
  else if(boardState[0][7]==1)
  {
    if(boardState[0][6]==1||boardState[0][8]==1)
    {
      return true;
    }
    if(boardState[0][4]==1||boardState[0][1]==1)
    {
      return true;
    }
  }
  else if(boardState[0][2]==1||boardState[0][4]==1||boardState[0][6]==1)
  {
    return true;
  }
  return false;
}

/*
 * updateLEDs() - sends the LED states set in ledStates to the 74HC595
 * sequence
 */
void updateLEDs(int value){
  digitalWrite(latch, LOW);     //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH);   //Pulls the latch high displaying the data
}


//These are used in the bitwise math that we use to change individual LEDs
//For more details http://en.wikipedia.org/wiki/Bitwise_operation
int bits[] = {B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};
int masks[] = {B11111110, B11111101, B11111011, B11110111, B11101111, B11011111, B10111111, B01111111};
/*
 * changeLED(int led, int state) - changes an individual LED 
 * LEDs are 0 to 7 and state is either 0 - OFF or 1 - ON
 */
 void changeLED(int led, int state){
   //ledState = ledState & masks[led];  //clears ledState of the bit we are addressing
   //ledState = ledState & B00000000;
   if(state == ON){ledState = ledState | bits[led];}
   if(state == OFF){
    ledState = ledState & masks[led];
   }
   updateLEDs(ledState);              //send the new LED state to the shift register
 }
